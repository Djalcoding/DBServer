
#include "client.h"
#include "logger.h"
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

struct Client::flush Client::flush = {};

Client::Client(Client &&moved)
    : file_descriptor(moved.file_descriptor),
      log_header(std::move(moved.log_header)),
      write_buffer(std::move(moved.write_buffer)), timeout(moved.timeout),
      last_packet_timestamp(moved.last_packet_timestamp) {
    moved.file_descriptor = -1;
}
Client &Client::operator=(Client &&moved) {
    file_descriptor = moved.file_descriptor;
    write_buffer = std::move(moved.write_buffer);
    log_header = std::move(moved.log_header);
    timeout = moved.timeout;
    last_packet_timestamp = moved.last_packet_timestamp;

    moved.file_descriptor = -1;

    return *this;
}

std::string Client::read_available() {
    ssize_t available_bytes = available();
    std::string buffer(available_bytes, '\0');
    read_n(available_bytes, buffer.data());
    return buffer;
}
ssize_t Client::read_n(ssize_t n, byte_t *buffer) {
    Logger::getInstance()->push(
        {log_header,
         std::format("Asking client to send (read operation) {} bytes ", n)});
    ssize_t out = ::read(file_descriptor, buffer, n);
    if (out == -1) {
        throw std::runtime_error("could not read file descriptor");
    } else if (out != 0)
        update_timer();

    return out;
}

bool Client::wait_for_data(int timeout) const {
    pollfd pfd{};
    pfd.fd = file_descriptor;
    pfd.events = POLLIN;
    return ::poll(&pfd, 1, timeout) > 0;
}
std::string Client::peek_available() const {
    ssize_t available_bytes = available();
    std::string buffer(available_bytes, '\0');
    peek_n(available_bytes, buffer.data());
    return buffer;
}
ssize_t Client::peek_n(ssize_t n, byte_t *buffer) const {
    Logger::getInstance()->push(
        {log_header,
         std::format("Asking client to show (peek operation) {} bytes ", n)});
    ssize_t out = recv(file_descriptor, buffer, n, MSG_PEEK);
    if (out == -1) // TODO : multiple tries
        throw std::runtime_error("could not read file descriptor");
    return out;
}

void Client::close() {
    ::shutdown(file_descriptor, SHUT_WR);
    ::close(file_descriptor);
}
ssize_t Client::available() const {
    ssize_t available_bytes = 0;
    ioctl(file_descriptor, FIONREAD, &available_bytes);
    return available_bytes;
}

