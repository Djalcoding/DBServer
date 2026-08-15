#pragma once
#include "segmented_string.h"
#include "server_types.h"
#include <alloca.h>
#include <asm-generic/socket.h>
#include <cassert>
#include <chrono>
#include <climits>
#include <format>
#include <poll.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <vector>

class Client {
    using packet_t = std::string_view;
    FD_T file_descriptor;
    std::string log_header;
    std::vector<iovec> write_buffer;
    using clock = std::chrono::steady_clock;
    using moment = std::chrono::time_point<clock>;
    moment last_packet_timestamp;
    clock::duration timeout;

  public:
    struct flush {
        std::vector<iovec> *buffer;
        flush() : buffer(nullptr) {};
        // the vector may be consumed
        flush(std::vector<iovec> *b) : buffer(b) {};
        flush operator()(std::vector<iovec> *b) {
            flush s;
            s.buffer = b;
            return s;
        };
    };
    static flush flush;
    Client(FD_T file_descriptor, int timeout = 60000)
        : file_descriptor(file_descriptor) {
        log_header = std::format("Client; fd:{}", file_descriptor);
        last_packet_timestamp = clock::now();
        this->timeout = std::chrono::milliseconds(timeout);
    };
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&);
    Client &operator=(Client &&);
    void close();
    [[nodiscard]]
    std::string read_available();
    template<std::size_t s>
    void read_available(segmented_string<s>& buf) {
        buf.read(file_descriptor);
    }
    ssize_t read_n(ssize_t n, byte_t *buffer);
    ssize_t peek_n(ssize_t n, byte_t *buffer) const;
    std::string peek_available() const;
    ssize_t available() const;
    void write(packet_t);
    void write_http(std::string_view response_type, packet_t contents);
    void write_http_ok(packet_t contents) { write_http("200 OK", contents); }
    bool wait_for_data(int timeout) const;
    bool stale() const {
        return (clock::now() - last_packet_timestamp) > timeout;
    };
    // everything after this is unsafe; no flush = maybe big crash
    friend Client &operator<<(Client &c, packet_t packet) {
        if (packet.empty())
            return c;
        c.write_buffer.push_back(
            iovec{const_cast<char *>(packet.data()), packet.size()});
        return c;
    }

    template <std::size_t s>
    friend Client &operator<<(Client &c, segmented_string<s> &string) {
        std::vector<char *> &buffers = string.data();
        c.write_buffer.reserve(c.write_buffer.size() + buffers.size());
        for (std::size_t i{0}; i < buffers.size(); i++) {
            std::size_t size = std::min(s, string.size() - i * s);
            c.write_buffer.push_back(
                iovec{(const_cast<char *>(buffers[i])), size});
        }
        return c;
    }
    friend void operator<<(Client &c, struct flush f) {
        std::vector<iovec> *write_buffer = &c.write_buffer;
    start:
        std::size_t start = 0;
        while (start < write_buffer->size()) {
            std::size_t buffers_to_write =
                std::min(write_buffer->size() - start,
                         static_cast<std::size_t>(IOV_MAX));
            ssize_t written_bytes =
                writev(c.file_descriptor, write_buffer->data() + start,
                       buffers_to_write);
            if (written_bytes == -1) {
                // TODO : Log
                return;
            }
            while (written_bytes > 0) {
                auto &buffer = (*write_buffer)[start];
                if (written_bytes >= buffer.iov_len) {
                    written_bytes -= buffer.iov_len;
                    start++;
                } else {
                    buffer.iov_len -= written_bytes;
                    buffer.iov_base =
                        static_cast<char *>(buffer.iov_base) + written_bytes;
                }
            }
        }
        write_buffer->clear();
        if (f.buffer != nullptr && f.buffer != write_buffer) {
            write_buffer = f.buffer;
            goto start;
        }
    }
};
