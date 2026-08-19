#pragma once
#include <concepts>
#include <ostream>
#include <type_traits>

template <class T>
concept Printable = requires(std::ostream &stream, T &t) {
    { stream << t } -> std::convertible_to<std::ostream &>;
};

template <Printable T>
std::ostream &operator<<(std::ostream &stream, const std::optional<T> &opt) {
    if (!opt.has_value())
        return stream << "<Empty Optional>";
    return stream << opt.value();
}

namespace concepts {

template <class Test, template <typename...> class Ref>
struct is_specialization : std::false_type {};

template <template <typename...> class Ref, typename... Args>
struct is_specialization<Ref<Args...>, Ref> : std::true_type {};
} // namespace concepts
