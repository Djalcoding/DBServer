#include "lib/client.h"
#include "lib/server.h"
#include "lib/servercache.h"
void cache();
int main() {
    cache();
    return 0;
}




void cache() {
    ServerBase server(72);
    server.start(10);
    Client client = server.accept_blocking();
    ServerCache<300> cache(600);
    client.wait_for_data(-1);
    auto& buf = cache.ask("Client", 458);
    std::cout << client.peek_available() << '\n';
    client.read_available(buf);
    std::cout << *cache.get("Client").value(); 

    //cache.display();
}

