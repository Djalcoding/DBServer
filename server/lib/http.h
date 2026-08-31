#pragma once
#include "servercache.h"
#include <optional>
#include <string_view>
#include <sys/types.h>

#define RN "\r\n"
namespace http {
template <class V>
concept ReadableView =
    (is_node_view<V>::value || std::same_as<std::string_view, V>);
template <class T>
concept UnsliceableReadableView =
    ReadableView<T> || std::convertible_to<T, std::string_view>;
using packet = const std::string_view;

struct HttpMethod {
    enum Value { GET = 0, POST = 1, PUT = 2, CONNECT = 3 };

  private:
    constexpr static std::size_t count = 4;
    static constexpr const char *names[count] = {"GET", "POST", "PUT",
                                                 "CONNECT"};
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

    template <ReadableView V> static constexpr HttpMethod fromString(V v) {
        for (std::size_t i{0}; i < count; i++) {
            if (v == names[i]) {
                return Value(i);
            }
        }
        throw std::invalid_argument("Non-existant name");
    }
};

class HttpReader {
    // make http reader not parse multiple times (so an actual datatype, lazily
    // evaluated)
    using http_packet_t = packet;

    template <ReadableView Packet, class... Args>
        requires((std::same_as<char, Args>), ...) // TODO : restrict this
    static constexpr std::size_t exhaust(Packet view, std::size_t &cursor,
                                         std::size_t n, Args... targets) {
        for (typename Packet::iterator it = view.begin() + cursor;
             n != 0 && it != view.end(); it++) {
            if (((view[cursor] == targets) || ...))
                n--;
            cursor++;
        }
        return cursor;
    }

  public:
    template <ReadableView Packet> static HttpMethod method(Packet packet) {
        return HttpMethod::fromString(
            packet.substr(0, packet.find_first_of(' ')));
    }
    template <ReadableView Packet> static Packet target(Packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 1, ' ');
        std::size_t end = exhaust(packet, cursor, 1, ' ');
        return packet.substr(start, end - start - 1);
    }
    template <ReadableView Packet> static Packet version(Packet packet) {
        std::size_t cursor = 0;
        std::size_t start = exhaust(packet, cursor, 2, ' ');
        std::size_t end = exhaust(packet, cursor, 1, '\r', '\n');
        return packet.substr(start, end - start - 1);
    }
    template <ReadableView Packet>
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
    template <ReadableView Packet> static Packet contents(Packet packet) {
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

template <ReadableView Packet> struct HttpRequest {
    const HttpMethod method;
    const Packet target;
    const Packet version;
    const Packet contents;
    const Packet raw_data;
    const std::optional<Packet> authorization;

    HttpRequest(Packet packet)
        : raw_data(packet), method(HttpReader::method(packet)),
          target(HttpReader::target(packet)),
          version(HttpReader::version(packet)),
          contents(HttpReader::contents(packet)),
          authorization(header("Authorization")) {}

    std::optional<const Packet> header(std::string_view name) {
        return HttpReader::header(
            raw_data,
            name); // I don't think caching here will have any real benefit
    }

    bool is(HttpMethod method) { return this->method == method; }
};

} // namespace http
