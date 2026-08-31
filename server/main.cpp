#include "lib/client.h"
#include "lib/filemanager.h"
#include "lib/http.h"
#include "lib/server.h"
#include <cassert>
#include <filesystem>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
    Server server{static_cast<unsigned int>(80)};
    const std::filesystem::path observed_directory{"./observed"};

    Server::Authorization refuse_all = [](auto) { return false; };
    FileManager fman{std::move(observed_directory)};

    static constexpr std::string HIERARCHY_ENDPOINT = "/hierarchy";
    using http::HttpReader;
    using http::HttpMethod::CONNECT;
    using http::HttpMethod::GET;
    using http::HttpMethod::POST;

    server.start(10);
    server.on(
              GET, "/", RESPOND_WITH(&) { client.write_http_ok("hi"); })
        .on(
            GET, "/status", RESPOND_WITH(&) { client.write_http_ok("Bye"); })
        .on(
            GET, "/cache",
            RESPOND_WITH(&) {
                std::stringstream ss;
                for (auto c : *server.getCache()) {
                    ss << c.second->key << " : " << c.second->size << "B \n";
                }
                client.write_http_ok(ss.str());
            })
        .on(
            GET, "/slow",
            RESPOND_WITH(&) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                client.write_http_ok("B y e");
            })
        .on(
            GET, HIERARCHY_ENDPOINT,
            RESPOND_WITH(&) {
                auto cache = server.getCache();
                if (auto cached_entry = cache->get(HIERARCHY_ENDPOINT)) {
                    client.write_http_ok(cached_entry.value());
                } else {
                    std::string hierarchy_display = fman.hierarchy_display();
                    client.write_http_ok(hierarchy_display);
                    cache->push(HIERARCHY_ENDPOINT, hierarchy_display);
                }
            })
        .on_predicate(
            [&](Server::http_packet &packet) {
                return packet.is(GET) && packet.target.starts_with("/stream/");
            },
            RESPOND_WITH(&){})
        .on_predicate(
            [&](Server::http_packet &packet) {
                return packet.is(GET) &&
                       fman.exist_within_subfilesystem(packet.target);
            },
            RESPOND_WITH(&) {
                client.sendfile(fman.get_full_path(packet.target));
            })
        .on_predicate(
            [](Server::http_packet &packet) { return packet.is(POST); },
            RESPOND_WITH(&) {
                std::filesystem::path path = packet.target;
                bool exists = fman.exist_within_subfilesystem(path);
                if (exists)
                    client.write_http("200 OK", "");
                else {
                    client.write_http("201 Created", "");
                    server.getCache()->remove(
                        HIERARCHY_ENDPOINT); // invalidate cached hierarchy
                                             // (todo; check if I can just
                                             // append the new file or if I can
                                             // just refetch)
                }
                fman.write(path, packet.contents);
            })
        .on_default(RESPOND_WITH() {
            client.write_http("404 Not Found", "Unknown page");
        });
    while (true) {
        server.accept_clients();
    }
}
