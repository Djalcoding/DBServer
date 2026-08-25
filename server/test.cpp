#include "lib/http.h"
#include "lib/server.h"
#include "lib/utils.h"
#include <barrier>
#include <sys/uio.h>
#include <thread>
#include <unistd.h>

void cache();

void thread(int id, std::barrier<> *barrier, ServerCache<100> *cache) {
    barrier->arrive_and_wait();
    cache->ask(std::format("", id), 200);
}

int main() {
    ServerCache<100> cache{20};
    std::barrier b(4);
    auto t1 = std::jthread{thread, 1, &b, &cache};
    auto t2 = std::jthread{thread, 2, &b, &cache};
    auto t3 = std::jthread{thread, 3, &b, &cache};
    auto t4 = std::jthread{thread, 4, &b, &cache};
    cache.display();
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
