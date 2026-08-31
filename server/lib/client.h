#pragma once
#include "http.h"
#include "logger.h"
#include "server_types.h"
#include "servercache.h"
#include <alloca.h>
#include <asm-generic/socket.h>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <type_traits>
#include <unistd.h>
#include <vector>

#define PACKET_TEMPLATE template <http::UnsliceableReadableView Packet>

class Client {
    FD_T file_descriptor;
    std::string log_header;
    std::vector<iovec> write_buffer;
    using clock = std::chrono::steady_clock;
    using moment = std::chrono::time_point<clock>;
    moment last_packet_timestamp;
    clock::duration timeout;
    inline static int enable_cork = 1;
    inline static int disable_cork = 0;

  public:
    struct flush {};
    static flush flush;
    Client(FD_T file_descriptor,
           int timeout = 60'000) // TODO: use std::chrono::seconds
        : file_descriptor(file_descriptor) {
        log_header = std::format("Client; fd:{}", file_descriptor);
        last_packet_timestamp = clock::now();
        this->timeout = std::chrono::milliseconds(timeout);
    };
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&);
    Client &operator=(Client &&);
    void cork() {
        ::setsockopt(file_descriptor, IPPROTO_TCP, TCP_CORK, &enable_cork,
                     sizeof(int));
    }
    void uncork() {
        ::setsockopt(file_descriptor, IPPROTO_TCP, TCP_CORK, &disable_cork,
                     sizeof(int));
    }
    void sendfile(const std::filesystem::path &path) {
        int file_opener = ::open(path.c_str(), O_RDONLY);
        cork();
        std::size_t filesize = std::filesystem::file_size(path);
        write_http_header("200 OK", filesize);
        ::sendfile(file_descriptor, file_opener, NULL,
                   std::filesystem::file_size(path));
        ::close(file_opener);
        uncork();
    }

    void close();
    [[nodiscard]]
    std::string read_available();
    ssize_t read_n(ssize_t n, byte_t *buffer);
    ssize_t readv(iovec *buffers, std::size_t buffer_c) {
        return ::readv(file_descriptor, buffers, buffer_c);
    }
    FD_T fd() { return file_descriptor; }

    template <std::size_t s>
    ServerCache<s>::Node &read(std::string_view key, ServerCache<s> *cache) {
        last_packet_timestamp = clock::now();
        auto &buf = cache->ask(key, available());
        ::iovec *b = new iovec[buf.owned()]; // TODO : handle zeroes
        buf.iovec(b);
        readv(b, buf.owned());
        return buf;
    }
    ssize_t peek_n(ssize_t n, byte_t *buffer) const;
    std::string peek_available() const;
    ssize_t available() const;

    template <http::UnsliceableReadableView Packet> void write(Packet input) {
        ssize_t remaining_bytes = input.size();
        while (remaining_bytes > 0) {
            int w = ::send(file_descriptor, input.data(), input.size(), 0);
            if (w == -1) {
                // TODO: Multiple attempts
                Logger::getInstance()->push(
                    {log_header,
                     std::format(
                         "Failed in writing {} bytes from packet, {} bytes "
                         "remaining",
                         w, remaining_bytes),
                     Logger::LogLevel::WAR});
                return;
            } else {
                remaining_bytes -= w;
            }
            Logger::getInstance()->push(
                {log_header,
                 std::format("sent {} byte packet", w, input.size())});
            update_timer();
        }
    }

    void write_http_header(std::string_view response_type, std::size_t length) {
        *this << "HTTP/1.1 " << response_type << RN
              << "Content-Length: " << std::to_string(length) << RN
              << "Connection : close" << RN << RN << Client::flush;
    }
    PACKET_TEMPLATE
    void write_http(std::string_view response_type, Packet contents,
                    bool keep_alive = false) {
        *this << "HTTP/1.1 " << response_type << "\r\n"
              << "Content-Length: ";
        if constexpr (std::is_pointer_v<Packet>) {
            *this << std::to_string(std::string_view(contents).size());
        } else {
            *this << std::to_string(contents.size());
        }
        *this << RN << "Connection : " << (keep_alive ? "keep-alive" : "close")
              << RN << RN << contents << Client::flush;
    }

    PACKET_TEMPLATE
    void write_http_ok(Packet contents) { write_http("200 OK", contents); }
    bool wait_for_data(int timeout) const;
    void update_timer() { last_packet_timestamp = clock::now(); }
    bool stale() const {
        return (clock::now() - last_packet_timestamp) > timeout;
    };

    bool peer_closed() const {
        pollfd pfd{};
        pfd.fd = file_descriptor;
        pfd.events = POLLRDHUP;
        return ::poll(&pfd, 1, 0);
    }

    // everything after this is unsafe; no flush = maybe big crash

    friend Client &operator<<(Client &c, std::string_view packet) {
        if (packet.empty())
            return c;
        c.write_buffer.push_back(
            iovec{const_cast<char *>(packet.data()), packet.size()});
        return c;
    }
    friend Client &operator<<(Client &c, const char *packet) {
        return c << std::string_view(packet);
    }
    template <NodeViewConcept NodeView>
    friend Client &operator<<(Client &c, NodeView view) {
        for (std::string_view buffer : view.buffers()) {
            c.write_buffer.push_back(
                {const_cast<char *>(buffer.data()), buffer.size()});
        }
        return c;
    }

    friend void operator<<(Client &c, struct flush) {
        // TODO: handle IOV_MAX
        ssize_t written = ::writev(c.file_descriptor, c.write_buffer.data(),
                                   c.write_buffer.size());
        c.write_buffer.clear();
    }
};
#undef PACKET_TEMPLATE
