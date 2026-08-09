#include "server.h"
#include "logger.h"
#include <cerrno>
#include <chrono>
#include <format>
#include <thread>
#include <unistd.h>
#define TRIAL_COUNT 5

void wait_ms(unsigned int time) { std::this_thread::sleep_for(std::chrono::milliseconds(time)); }

ServerBase::ServerBase(unsigned int port) : port(port) {
    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd == -1)
        throw std::runtime_error("Failed to assign socket");
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    log_header = std::format("SERVER:{}; fd:{}", port, server_socket_fd);
    int remaining_trial = TRIAL_COUNT;
    while (bind(server_socket_fd, (sockaddr *)&address, sizeof(address)) == -1 && remaining_trial--) {
        Logger::getInstance()->push({log_header,
                                     std::format("Failed to bind on port {} (err {}), retrying in 3 s [{}/{}]", port,
                                                 errno, remaining_trial, TRIAL_COUNT),
                                     Logger::LogLevel::WAR});
        wait_ms(3000);
    }
    if (remaining_trial <= 0) {
        Logger::getInstance()->push({log_header,
                                     std::format("Could not bind on port {}, gracefully shutting down", port),
                                     Logger::LogLevel::ERR});
    } else {
        Logger::getInstance()->push({log_header, std::format("Binded to port {}", port)});
    }
}

ServerBase::~ServerBase() { close(server_socket_fd); }

bool ServerBase::start(unsigned int backlog_size) noexcept(false) {
    if (started) {
        Logger::getInstance()->push({log_header,
                                     std::format("tried to start but it was already started on port {}", port),
                                     Logger::LogLevel::WAR});
        return false;
    }
    listen(server_socket_fd, backlog_size);
    Logger::getInstance()->push(
        {log_header, std::format("Started on port {}, on file descriptor {}", port, server_socket_fd)});
    started = true;
    return true;
}

std::optional<Client> ServerBase::accept(int timeout) {
    pollfd pollfd = {};
    pollfd.fd = server_socket_fd;
    pollfd.events = POLLIN;
    if (poll(&pollfd, 1, timeout) <= 0)
        return std::nullopt;
    return accept_blocking();
}
Client ServerBase::accept_blocking() {
    FD_T client_socket_fd = ::accept(server_socket_fd, nullptr, nullptr);
    assert(client_socket_fd != -1);
    return Client{client_socket_fd};
}

Server &Server::on_default(ServerResponse response) {
    default_response = response;
    return *this;
}


bool Server::accept_clients(int timeout) {
    if (std::optional<Client> client = ServerBase::accept(timeout)) {
        return pool.execute(std::packaged_task<void()>([this, client = std::move(*client)]() mutable {
            client.wait_for_data(3000);
            const std::string request = client.peek_available();
            for (auto &route : routes) {
                if (route.predicate(request)) {
                    route.response(client);
                    goto close_client;
                    return;
                }
            }
            default_response(client);
        close_client:
            client.close();
        }));
    } else
        return false;
}
