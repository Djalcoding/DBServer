#pragma once
#include "logger.h"
#include "server_types.h"
#include <alloca.h>
#include <asm-generic/socket.h>
#include <cassert>
#include <format>
#include <poll.h>
#include <sstream>
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
    struct flush {};
    using packet_t = std::string_view;
    FD_T               file_descriptor;
    std::string        log_header;
    std::stringstream  stream;
    std::vector<iovec> write_buffer;

  public:
    static flush flush;
    Client(FD_T file_descriptor, int timeout = 3) : file_descriptor(file_descriptor) {
        log_header = std::format("Client; fd:{}", file_descriptor);
        linger linger_opt;
        linger_opt.l_onoff = 1;
        linger_opt.l_linger = timeout;
        if (setsockopt(file_descriptor, SOL_SOCKET, SO_LINGER, &linger_opt, sizeof(linger_opt)) < 0) {
            Logger::getInstance()->push({"setsockopt", errno});
        }
    };
    Client(const Client &) = delete;
    Client &operator=(const Client &) = delete;
    Client(Client &&);
    Client &operator=(Client &&);
    //~Client();
    void close();
    [[nodiscard]]
    std::string    read_available();
    ssize_t        read_n(ssize_t n, byte_t *buffer);
    ssize_t        peek_n(ssize_t n, byte_t *buffer) const;
    std::string    peek_available() const;
    ssize_t        available() const;
    void           write(packet_t);
    void           write_http(std::string_view response_type, packet_t contents);
    void           write_http_ok(packet_t contents) { write_http("200 OK", contents); }
    bool           wait_for_data(int timeout) const;
    friend Client &operator<<(Client &c, packet_t packet) {
        c.write_buffer.push_back(iovec{&const_cast<char &>(packet[0]), packet.size()});
        return c;
    }
    friend void operator<<(Client &c, struct flush) {
        writev(c.file_descriptor, c.write_buffer.data(), c.write_buffer.size());
        c.write_buffer.clear();
    }
};
