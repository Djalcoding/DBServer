#include "client.h"
#include "filereader.h"
#include "utils.h"
#include <string_view>
#include <sys/types.h>
#pragma once

namespace http {
template <class V>
concept View = (is_node_view<V>::value || std::same_as<std::string_view, V>);
using packet = const std::string_view;

struct HttpMethod {
    enum Value { GET = 0, POST = 1, PUT = 2 };

  private:
    constexpr static std::size_t count = 3;
    static constexpr const char *names[count] = {"GET", "POST", "PUT"};
    Value v;

  public:
    constexpr HttpMethod(const Value &v) : v(v) {}
    constexpr HttpMethod &operator=(const Value &v) {
        this->v = v;
        return *this;
    }
    constexpr bool operator==(const HttpMethod &other) const {
        return other.v == this->v;
    }
    constexpr bool operator!=(const HttpMethod &other) const {
        return other.v != this->v;
    }
    constexpr const char *toString() const { return names[v]; }

    template <View V> static constexpr HttpMethod fromString(V v) {
        for (std::size_t i{0}; i < count; i++) {
            if (v == names[i]) {
                return Value(i);
            }
        }
        throw std::invalid_argument("Non-existant name");
    }
};
// TODO re-add GET macro

class HttpReader {
    using http_packet_t = packet;

    template <View Packet, class... Args>
        requires((std::same_as<char, Args>), ...) // TODO : restrict this
    static constexpr std::size_t exhaust(Packet view, std::size_t &cursor,
                                         std::size_t n, Args... targets) {
        for (typename Packet::iterator it = view.begin() + cursor; n != 0 && it != view.end();
             it++) {
            std::cout << "Checking " << cursor << " : " << view[cursor] << '\n';
            if (((view[cursor] == targets) || ...))
                n--;
            cursor++;
        }
        return cursor;
    }

  public:
    template <View Packet>
    static HttpMethod method(Packet packet) {
        return HttpMethod::fromString(
            packet.substr(0, packet.find_first_of(' ')));
    }
    template <View Packet> static Packet target(Packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 1, ' ');
        std::cout << start << '\n';
        std::size_t end = exhaust(packet, cursor, 1, ' ');
        std::cout << end << '\n';
        return packet.substr(start, end - start - 1);
    }
    template <View Packet> static Packet version(Packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 2, ' ');
        std::size_t end = exhaust(packet, cursor, 1, '\r', '\n');
        return packet.substr(start, end - start - 1);
    }
    template <View Packet>
    static std::optional<Packet> header(Packet packet,
                                        std::string_view target) {
        std::size_t cursor = 0;
        Packet view = packet;
        while (view != target) {
            std::size_t line_start = exhaust(packet, cursor, 2, '\r', '\n');
            if (cursor >= packet.size())
                return std::nullopt;
            view = packet.substr(line_start, exhaust(packet, cursor, 1, ':') -
                                                 line_start - 1);
        }
        std::size_t start = cursor + 1;
        return packet.substr(start,
                             exhaust(packet, cursor, 1, '\r') - start - 1);
    }
    template <View Packet> static std::string_view contents(Packet packet) {
        std::size_t cursor = 0;
        int s = 0;
        int e = 0;
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
