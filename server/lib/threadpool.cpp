#include "threadpool.h"

Worker::Worker() {
    thread = std::jthread([this](std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            {
                std::unique_lock lock(mutex);
                sleeper.wait(lock, [&, this] { return is_busy() || stop_token.stop_requested(); });
            }
            try {
                process();
            } catch (...) {
            }
            busy.store(false);
        }
    });
}
Worker::~Worker() {
    thread.request_stop();
    sleeper.notify_one();
}
void Worker::execute(function_t &&func) {
    if (busy)
        throw std::runtime_error("Tried to execute function on working thread");
    busy.store(true);
    process = std::move(func);
    sleeper.notify_one();
}

bool ThreadPool::execute(Worker::function_t function) {
    for (Worker &worker : workers) {
        if (worker.is_busy())
            continue;
        worker.execute(std::move(function));
        return true;
    }
    return false;
}
std::size_t ThreadPool::available() {
    std::size_t count = 0;
    for (Worker &worker : workers) {
        count += !worker.is_busy();
    }
    return count;
}
