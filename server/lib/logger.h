#pragma once

#include <cstring>
#include <ctime>
#include <iostream>
#include <queue>
#include <sstream>
#include <string>
#include <thread>
#define PROGRAM_NAME "HTTP_SERVER"
class Logger {
  public:
    enum class LogLevel : uint8_t {
        INF = 0,
        WAR = 1,
        ERR = 2,
    };

  private:
    constexpr static char const *log_level_to_str[] = {"[INF]", "[WAR]", "[ERR]"};
    class Log {
        const std::string header;
        const std::string contents;
        const LogLevel    log_level;
        const std::time_t timestamp;

      public:
        Log(std::string header, std::string contents, LogLevel level = LogLevel::INF)
            : header(header), contents(contents), log_level(level),
              timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}
        Log(std::string header, int err, LogLevel level = LogLevel::INF)
            : header(header), contents(std::strerror(err)), log_level(level),
              timestamp(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())) {}
        friend std::stringstream &operator<<(std::stringstream &stream, const Log &log) {
            char date_buffer[100];
            std::strftime(date_buffer, sizeof(date_buffer), "%A %c", std::localtime(&log.timestamp));
            stream << log_level_to_str[static_cast<uint8_t>(log.log_level)] << "[" << date_buffer << "] ["
                   << PROGRAM_NAME << "; " << log.header << "] " << log.contents << std::endl;
            return stream;
        }
    };

    using message_t = Log;
    static Logger        *instance;
    std::jthread          thread;
    std::queue<message_t> message_queue;
    std::stringstream     string_buffer;

    Logger() {
        thread = std::jthread([&](std::stop_token token) {
            while (!token.stop_requested()) {
                string_buffer.str("");
                while (!message_queue.empty()) {
                    string_buffer << message_queue.front();
                    message_queue.pop();
                }
                std::cout << string_buffer.str();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        });
    }

  public:
    static Logger *getInstance() {
        if (!instance) {
            instance = new Logger();
        }
        return instance;
    };

    void push(const Log &log) { message_queue.push(log); }
};
