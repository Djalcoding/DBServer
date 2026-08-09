#pragma once

#include "client.h"
#include "http.h"
#include "server_types.h"
#include "servercache.h"
#include "threadpool.h"
#include <cassert>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <netinet/in.h>
#include <optional>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class ServerBase {
  protected:
    FD_T               server_socket_fd;
    const unsigned int port;
    bool               started = false;
    std::string        log_header;

  public:
    ServerBase(unsigned int port) noexcept(false);
    ServerBase(const ServerBase &) = delete;
    ServerBase &operator=(const ServerBase &) = delete;
    ServerBase(ServerBase &&) = delete;
    ServerBase &operator=(ServerBase &&) = delete;
    ~ServerBase();
    bool                  start(unsigned int backlog_size) noexcept(false);
    std::optional<Client> accept(int timeout);
    [[nodiscard]] Client  accept_blocking();
};

class Server : private ServerBase {
    using ServerResponse = std::function<void(Client &)>;
    using Predicate = std::function<bool(http::packet)>;
    using Cache_t = ServerCache<100>;
    struct ServerRoute {
        Predicate      predicate;
        ServerResponse response;
        ServerRoute(Predicate predicate, ServerResponse response)
            : predicate(std::move(predicate)), response(std::move(response)) {};
    };
    using ServerRequest = std::shared_ptr<std::packaged_task<void()>>;
    ServerResponse           default_response = [](Client &) {};
    std::vector<ServerRoute> routes;
    ThreadPool               pool{20};
    Cache_t                  cache;
    /// Returns true if a client was accepted
  public:
    Server(unsigned int port) : ServerBase(port) {};
    void    start(unsigned int backlog_size = 10) { ServerBase::start(backlog_size); }
    Server &on_default(ServerResponse response);
    Server &on(const std::string &target, ServerResponse response) { return on(GET, target, response); }
    Server &on(HttpMethod method, const std::string &target, ServerResponse response) {
        return on_predicate(
            [=](std::string_view packet) {
                return method == http::HttpReader::method(packet) && target == http::HttpReader::target(packet);
            },
            response);
    }
    Server &on_predicate(Predicate pred, ServerResponse response) {
        routes.push_back({pred, response});
        return *this;
    }
    Cache_t *getCache() { return &cache; }
    bool     accept_clients(int timeout = 10);
};
