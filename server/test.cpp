#include "lib/http.h"
#include "lib/server.h"
#include <sys/uio.h>
#include <unistd.h>
#include "lib/utils.h"

void cache();
int main() {
    cache();
    return 0;
}

void cache() {
    ServerBase server(8080);
    server.start(10);
    Client c = server.accept_blocking();
    ServerCache<100> cache{20};
    c.wait_for_data(-1);
    auto n = c.read("Client", &cache);
    decltype(n)::view view = n;
    std::cout << "Method : " << http::HttpReader::method(view).toString()<< '\n';
    std::cout << "Info : " << http::HttpReader::target(view)<< '\n';
    std::cout << "Version : " << http::HttpReader::version(view)<< '\n';
    std::cout << "Version : " << http::HttpReader::header(view, "Connection") << '\n';
}
