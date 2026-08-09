#include <condition_variable>
#include <future>
#include <mutex>
#include <thread>
#include <vector>
#pragma once
struct Worker {
    using function_t = std::packaged_task<void()>;

  private:
    std::jthread            thread;
    function_t              process = function_t([] {});
    std::atomic<bool>       busy = false;
    std::condition_variable sleeper;
    std::mutex              mutex;

  public:
    Worker();
    ~Worker();
    bool is_busy() const { return busy.load(); }
    void execute(function_t &&func);
};

class ThreadPool {
    std::vector<Worker> workers;

  public:
    ThreadPool(std::size_t size) : workers(size) {}
    /// Returns true if process was sent to worker thread
    bool        execute(Worker::function_t function);
    std::size_t available();
};
