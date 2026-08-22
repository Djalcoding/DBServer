#include "lib/client.h"
#include "lib/http.h"
#include "lib/server.h"
#include <cassert>
#include <filesystem>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char **argv) {
    Server server{static_cast<unsigned int>(9999)};
    const std::filesystem::path observed_directory{
        "./observed"}; // TODO : Add str var
    assert(std::filesystem::is_directory(observed_directory));
    server.start(10);
    server
        .on(http::HttpMethod::GET, "/",
            [](Client &c, Server::packet) { c.write_http_ok("hi"); })
        .on(http::HttpMethod::GET, "/status",
            [](Client &c, Server::packet) { c.write_http_ok("Bye"); })
        /*.on(GET, "/cache",
            [&](Client &c, http::packet) {
                std::stringstream ss;
                for (auto c : *server.getCache()) {
                    if (c.empty())
                        continue;
                    ss << std::filesystem::path(c.key)
                              .lexically_relative(observed_directory)
                              .native()
                       << " : " << c.value.size() << "B \n";
                }
                c.write_http_ok(ss.str());
            })*/
        .on(http::HttpMethod::GET, "/slow",
            [](Client &c, Server::packet) {
                std::this_thread::sleep_for(std::chrono::seconds(10));
                c.write_http_ok("B y e");
            })
        .on(http::HttpMethod::GET, "/hierarchy",
            [&](Client &c, Server::packet) {
                std::stringstream stream;
                for (auto &object :
                     std::filesystem::recursive_directory_iterator(
                         observed_directory)) {
                    stream << object.path().generic_string().erase(
                                  0, observed_directory.generic_string().size())
                           << "\r\n";
                }
                c.write_http_ok(stream.str());
            })
        .on_predicate(
            [&](Server::packet packet) {
                std::filesystem::path p =
                    observed_directory /
                    http::HttpReader::target(packet).substr(
                        1); 
                return std::filesystem::exists(p) &&
                       std::filesystem::is_regular_file(p) &&
                       http::HttpReader::method(packet) ==
                           http::HttpMethod::GET &&
                       !std::filesystem::canonical(p)
                            .lexically_relative(
                                std::filesystem::canonical(observed_directory))
                            .generic_string()
                            .starts_with("..");
            },
            [&](Client &c, Server::packet request) {
                std::filesystem::path path =
                    observed_directory /
                    http::HttpReader::target(request).substr(1);
                c.sendfile(path);
            })
        .on_default([](Client &c, Server::packet) {
            c.write_http("404 Not Found", "Unknown page");
        });
    while (true) {
        server.accept_clients();
    }
}
