#pragma once

#include "client.h"
#include "http.h"
#include "server_types.h"
#include "servercache.h"
#include "threadpool.h"
#include <cassert>
#include <functional>
#include <future>
#include <memory>
#include <netinet/in.h>
#include <optional>
#include <sys/poll.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

class ServerBase {
  protected:
    FD_T server_socket_fd;
    const unsigned int port;
    bool started = false;
    std::string log_header;

  public:
    ServerBase(unsigned int port) noexcept(false);
    ServerBase(const ServerBase &) = delete;
    ServerBase &operator=(const ServerBase &) = delete;
    ServerBase(ServerBase &&) = delete;
    ServerBase &operator=(ServerBase &&) = delete;
    ~ServerBase();
    bool start(unsigned int backlog_size) noexcept(false);
    std::optional<Client> accept(int timeout);
    [[nodiscard]] Client accept_blocking();
};

class Server : private ServerBase {
#define RESPOND_WITH(CAPTURE)                                                  \
    [CAPTURE](Client & client, Server::http_packet & packet)

    using Cache_t = ServerCache<65536>;

  public:
    using packet = Cache_t::NodeView;
    using http_packet = http::HttpRequest<packet>;

    using Authorization = std::function<bool(std::optional<packet>)>;
    inline static Authorization any_auth = [](auto) { return true; };

  private:
    using ServerResponse = std::function<void(Client &, http_packet &)>;
    using Predicate = std::function<bool(http_packet &)>;

    struct ServerRoute {
        Predicate predicate;
        ServerResponse response;
        Authorization &auth;
        ServerRoute(Predicate predicate, ServerResponse response,
                    Authorization &auth)
            : predicate(predicate), response(response), auth(auth) {};
    };
    using ServerRequest = std::shared_ptr<std::packaged_task<void()>>;
    ServerResponse default_response = [](Client &, http_packet) {};
    ServerResponse unauthorized_response = [](Client &client, http_packet) {
        client.write_http("403 Forbidden", "");
    };
    std::vector<ServerRoute> routes;
    ThreadPool pool{8};
    Cache_t cache;
    unsigned long long connection_count = 0;

  public:
    Server(unsigned int port) : ServerBase(port), cache(1000) {};
    void start(unsigned int backlog_size = 10) {
        ServerBase::start(backlog_size);
    }
    Server &on_default(ServerResponse response);
    Server &on_unauthorized(ServerResponse response) {
        unauthorized_response = response;
        return *this;
    }
    Server &on(const std::string &target, ServerResponse response,
               Authorization &auth = any_auth) {
        return on(http::HttpMethod::GET, target, response, auth);
    }
    Server &on(http::HttpMethod method, const std::string &target,
               ServerResponse response, Authorization &auth = any_auth) {
        return on_predicate(
            [=](http_packet packet) {
                return method == packet.method && target == packet.target;
            },
            response, auth);
    }
    Server &on_predicate(Predicate pred, ServerResponse response,
                         Authorization &auth = any_auth) {
        routes.push_back({pred, response, auth});
        return *this;
    }
    Cache_t *getCache() { return &cache; }
    bool accept_clients(int timeout = 10);
};
