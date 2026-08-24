#include "lib/http.h"
#include "lib/server.h"
#include "lib/utils.h"
#include <sys/uio.h>
#include <unistd.h>

void cache();
int main() {
    cache();
    return 0;
}

void cache() {
    ServerCache<100> cache{20};
    cache.ask("Ask 4", 462);
    cache.display();
    cache.remove("Ask 4");
    cache.display();
    cache.ask("Ask 3", 68).owned();
    cache.display();
    cache.remove("Ask 3");
    auto n = cache.ask("Ask 4", 462);
    cache.display();
    std::cout << "Child child : " << n.child->child;
}
