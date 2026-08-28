#include "lib/filemanager.h"
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
    cache->ask(std::format("{}", id), 200);
}

int main() {
    FileManager::test();
    return 0;
}

