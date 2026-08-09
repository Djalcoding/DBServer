#include "client.h"
#include "filereader.h"
#include <string_view>
#include <sys/types.h>
#pragma once

struct HttpMethod {
    enum Value { GET = 0, POST = 1, PUT = 2 };

  private:
    constexpr static std::size_t count = 3;
    static constexpr const char *names[count] = {"GET", "POST", "PUT"};
    Value                        v;

  public:
    constexpr HttpMethod(const Value &v) : v(v) {}
    constexpr HttpMethod &operator=(const Value &v) {
        this->v = v;
        return *this;
    }
    constexpr bool              operator==(const HttpMethod &other) { return other.v == this->v; }
    constexpr bool              operator!=(const HttpMethod &other) { return other.v != this->v; }
    constexpr const char       *toString() { return names[v]; }
    static constexpr HttpMethod fromString(std::string_view v) {
        for (std::size_t i{0}; i < count; i++) {
            if (v == names[i]) {
                return Value(i);
            }
        }
        throw std::invalid_argument("Non-existant name");
    }
};

#define GET                                                                                                            \
    HttpMethod { HttpMethod::GET }
namespace http {
using packet = const std::string_view;
class HttpReader {
    using http_packet_t = packet;

    template <class... Args>
        requires((std::same_as<char, Args>), ...)
    static constexpr std::size_t exhaust(std::string_view view, std::size_t &cursor, std::size_t n, Args... targets) {
        while (n != 0) {
            if (((view[cursor] == targets) || ...))
                n--;
            cursor++;
            if (cursor >= view.size())
                return cursor;
        }
        return cursor;
    }

  public:
    static HttpMethod method(packet packet) {
        return HttpMethod::fromString(packet.substr(0, packet.find_first_of(' ')));
    }
    static std::string_view target(packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 1, ' ');
        std::size_t end = exhaust(packet, cursor, 1, ' ');
        return packet.substr(start, end - start - 1);
    }
    static std::string_view version(packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 2, ' ');
        std::size_t end = exhaust(packet, cursor, 1, '\r', '\n');
        return packet.substr(start, end - start - 1);
    }
    static std::optional<std::string_view> header(packet packet, std::string_view target) {
        std::size_t      cursor = 0;
        std::string_view view;
        while (view != target) {
            std::size_t line_start = exhaust(packet, cursor, 2, '\r', '\n');
            if (cursor >= packet.length())
                return std::nullopt;
            view = packet.substr(line_start, exhaust(packet, cursor, 1, ':') - line_start - 1);
        }
        std::size_t start = cursor + 1;
        return packet.substr(start, exhaust(packet, cursor, 1, '\r') - start);
    }
    static std::string_view contents(packet packet) {
        std::size_t cursor = 0;
        int         s = 0;
        int         e = 0;
        while (e - s != 2) {
            s = cursor;
            e = exhaust(packet, cursor, 2, '\r', '\n');
        }
        return packet.substr(cursor, packet.size() - cursor);
    }
};


inline FileReader send_file(const std::filesystem::path &path, Client &client) {
    FileReader reader{path};
    client.write_http_ok(reader.get_contents());
    return reader;
}
} // namespace http
