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
    FileManager fman{std::move(observed_directory)};
    assert(std::filesystem::is_directory(observed_directory));
    server.start(10);
    server
        .on(http::HttpMethod::GET, "/",
            [](Client &c, Server::packet) { c.write_http_ok("hi"); })
        .on(http::HttpMethod::GET, "/status",
            [](Client &c, Server::packet) { c.write_http_ok("Bye"); })
        .on(http::HttpMethod::GET, "/cache",
            [&](Client &c, Server::packet) {
                std::stringstream ss;
                for (auto c : *server.getCache()) {
                    ss << std::filesystem::path(c.second->key)
                              .lexically_relative(observed_directory)
                              .native()
                       << " : " << c.second->size << "B \n";
                }
                c.write_http_ok(ss.str());
            })
        .on(http::HttpMethod::GET, "/slow",
            [](Client &c, Server::packet) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                c.write_http_ok("B y e");
            })
        .on(http::HttpMethod::GET, "/hierarchy",
            [&](Client &c, Server::packet) {
                c.write_http_ok(fman.hierarchy_display());
            })
        .on_predicate(
            [&](Server::packet packet) {
                return http::HttpReader::method(packet) ==
                           http::HttpMethod::GET &&
                       fman.exist_within_subfilesystem(
                           http::HttpReader::target(packet));
            },
            [&](Client &c, Server::packet packet) {
                c.sendfile(
                    fman.get_full_path(http::HttpReader::target(packet)));
            })
        .on_predicate(
            [&](Server::packet packet) {
                return http::HttpReader::method(packet) ==
                       http::HttpMethod::POST;
            },
            [&](Client &c, Server::packet packet) {
                std::filesystem::path path = http::HttpReader::target(packet);
                bool exists = fman.exist_within_subfilesystem(path);
                if (exists)
                    c.write_http("200 OK", "");
                else
                    c.write_http("201 Created", "");
                fman.write(path, http::HttpReader::contents(packet));
            })
        .on_default([](Client &c, Server::packet) {
            c.write_http("404 Not Found", "Unknown page");
        });
    while (true) {
        server.accept_clients();
    }
}
