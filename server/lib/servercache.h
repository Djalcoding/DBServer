// TODO : seperate in files

#pragma once
#include <bits/xopen_lim.h>
#include <concepts>
#include <cstdlib>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <sys/uio.h>
#include <type_traits>
#include <unordered_map>

namespace {
template <class T>
concept NodeViewConcept = requires { typename T::node_view_tag; };

} // namespace
template <class T> struct is_node_view : std::false_type {};
template <NodeViewConcept C> struct is_node_view<C> : std::true_type {};
// LRU
template <std::size_t bs>
    requires(bs >= 1)
class ServerCache {
  public:
    static constexpr std::size_t buffer_size = bs;
    struct NodeView;
    struct Node {
        using view = NodeView;
        struct Iterator {
            std::size_t idx;
          private:
            Node *node;
            Node *start;

            void move_forward_one() {
                if (!start->parent() ||
                    start->parent() && node == start->child->child) {
                    node = nullptr;
                } else {
                    node = node->next_buffer();
                    idx = 0;
                }
            };

            void move_back_one() {
                idx = buffer_size - 1;
                if (node == start)
                    node = nullptr;
                else if (node->prev == nullptr) {
                    node = start;
                } else {
                    node = node->prev;
                }
            };

          public:
            Iterator(std::size_t idx, Node *node, Node *start)
                : idx(idx), node(node), start(start) {

                  };
            Iterator operator--() {
                if (idx == 0) {
                    move_back_one();
                } else
                    idx--;
                return *this;
            }
            Iterator operator--(int) {
                Iterator copy = *this;
                --(*this);
                return copy;
            }
            Iterator operator++() {
                idx++;
                if (idx == buffer_size) {
                    move_forward_one();
                }
                return *this;
            }
            Iterator operator++(int) {
                Iterator copy = *this;
                ++(*this);
                return copy;
            }

            Iterator operator+(std::size_t offset) const {
                Iterator cpy = *this;
                cpy += offset;
                return cpy;
            }
            Iterator operator-(std::size_t offset) const {
                Iterator cpy = *this;
                cpy -= offset;
                return cpy;
            }

            Iterator &operator-=(std::size_t count) {
                std::size_t jumps = (count + idx) / buffer_size;
                while (jumps--) {
                    move_back_one();
                }
                idx = (count + idx) % buffer_size;
                return *this;
            }
            Iterator &operator+=(std::size_t count) {
                std::size_t jumps = (count + idx) / buffer_size;
                while (jumps--) {
                    move_forward_one();
                }
                idx = (count + idx) % buffer_size;
                return *this;
            }
            bool operator==(const Iterator &other) {
                if (node == nullptr && other.node == nullptr)
                    return true;
                return idx == other.idx && node == other.node;
            }
            bool operator!=(const Iterator &other) { return !(*this == other); }
            char operator*() const { return node->data[idx]; }
            friend struct NodeView;
        };

        using ptr = Node *;
        using iterator = Iterator;
        std::string key;
        std::size_t size = 0;

        Iterator end() {
            std::size_t idx = size % buffer_size;
            if (!parent()) {
                return idx == 0 ? Iterator{0, nullptr, this}
                                : Iterator{idx, this, this};
            }
            return idx == 0 ? Iterator{0, nullptr, this}
                            : Iterator{idx, this->child->child, this};
        };
        Iterator begin() { return Iterator{0, this, this}; }
        // TODO rbegin and rend

      private:
        ptr next;
        ptr prev;
        ptr child;

        bool has_data = false;
        char data[buffer_size];
        Node(ptr prev, const std::string &key, ptr next)
            : prev(prev), key(key), next(next) {}

        /// expand the node and set the has_data flag to false, this should be
        /// paired with removing the key from the map
        ptr evict() {
            has_data = false;
            if (!parent())
                return this;
            ptr end = next;
            ptr childTail = child->child;
            next = child;
            child->prev = this;
            childTail->next = end;
            end->prev = childTail;
            child->child = nullptr;
            child = nullptr;
            return childTail;
        }

        Node *next_buffer() {
            if (this->parent())
                return this->child;
            else
                return this->next;
        }

        std::size_t real_size() const { return owned() * buffer_size; }

        friend class ServerCache;

      public:
        template <class Self>
        auto buffer(this Self &&s, std::size_t n = 0) -> decltype(&s.data[0]) {
            ptr curr = &s;
            while (n--) {
                curr = curr->next_buffer();
                if (!curr)
                    throw std::overflow_error(
                        std::format("buffer {} doesn't exist", n));
            }
            return curr->data;
        }
        template <class Self>
        auto operator[](this Self &&s, std::size_t idx) -> decltype(s.data[0]) {
            return s.buffer(idx / buffer_size)[idx % buffer_size];
        };
        bool parent() const { return child != nullptr && has_data; };
        std::size_t owned() const {
            if (!parent())
                return 1;
            return (size / buffer_size) + (size % buffer_size != 0);
        }

        // returns true if it has written all buffers or if it has hit the limit
        bool iovec(struct iovec *iov_buffer) {
            std::size_t cursor = 0;
            ptr curr = this;
            std::size_t i{0};
            std::size_t iterations =
                std::min(static_cast<std::size_t>(IOV_MAX), owned());
            while (i < iterations) {
                iov_buffer[i] = ::iovec{curr->buffer(), buffer_size};
                curr = curr->next_buffer();
                i++;
            }
            return i == owned();
        }

        friend std::ostream &operator<<(std::ostream &stream,
                                        const Node &node) {
            stream.write(node.data, std::min(buffer_size, node.size));
            if (!node.parent())
                return stream;
            NodePtr curr = node.child;
            while (curr) {
                if (curr->next)
                    stream.write(curr->data, buffer_size);
                else
                    stream.write(curr->data,
                                 buffer_size - (node.size % buffer_size));
                curr = curr->next_buffer();
            }
            return stream;
        }
    };
    struct NodeView {
        using node_view_tag = void;
        using Iterator = Node::Iterator;
        Iterator start_it;
        Iterator end_it;
        std::size_t length;
        std::size_t size() { return length; };
        using iterator = Iterator;
        NodeView(Node node)
            : start_it(node.begin()), end_it(node.end()), length(node.size) {}

        NodeView &operator=(const NodeView &view) = default;
        const Iterator begin() const { return start_it; }
        const Iterator end() const { return end_it; }
        char operator[](std::size_t idx) const {
            if (idx >= length) {
                throw std::overflow_error(
                    "Overflow ! "); // TODO : better error message (or just
                                    // build -fno-exceptions)
            }
            return *(start_it + idx);
        };

        NodeView substr(std::size_t start, std::size_t length) {
            if (start + length > this->length) {
                throw std::overflow_error(
                    "Overflow ! "); // TODO : better error message (or just
                                    // build -fno-exceptions)
            }
            return NodeView{start_it + start, start_it + start + length,
                            length};
        }
        std::size_t find_first_of(char target) {
            std::size_t i{0};
            for (char c : *this) {
                if (c == target)
                    return i;
                i++;
            }
            return i;
        }

        bool operator==(std::string_view other) {
            if (length != other.size())
                return false;
            Iterator s1 = this->begin();
            auto s2 = other.begin();
            while (s1 != end()) {
                if (*s1 != *s2) {
                    return false;
                }
                s1++;
                s2++;
            }
            return true;
        }
        bool operator==(const NodeView &other) {
            if (length != other.length)
                return false;
            Iterator s1 = begin();
            Iterator s2 = other.begin();
            while (s1 != end()) {
                if (*s1 != *s2)
                    return false;
                s1++;
                s2++;
            }
            return true;
        }

        friend std::ostream &operator<<(std::ostream &stream,
                                        const NodeView &view) {
            for (char c : view) {
                std::cout << c;
            }
            return stream;
        }

      private:
        NodeView(Iterator start, Iterator end, std::size_t length)
            : start_it(start), end_it(end), length(length) {}
    };

  private:
    struct Hasher {
        using is_transparent = void;
        std::size_t operator()(std::string_view str) const {
            return std::hash<std::string_view>{}(str);
        }
    };
    using NodePtr = Node::ptr;
    NodePtr head;
    NodePtr tail;
    using map_t =
        std::unordered_map<std::string, NodePtr, Hasher, std::equal_to<>>;
    map_t map;

  public:
    ServerCache(std::size_t node_count) {
        if (node_count < 2)
            throw std::invalid_argument("The cache requires at least 2 nodes");
        head = new Node(NodePtr(nullptr), "", NodePtr(nullptr));
        NodePtr prev = head;
        NodePtr curr;
        for (std::size_t i{0}; i < node_count - 1; i++) {
            curr = new Node(NodePtr(prev), "", NodePtr(nullptr));
            prev->next = curr;
            prev = curr;
        }
        tail = curr;
    }

    ~ServerCache() { // TODO : change this
        NodePtr curr = head;
        while (curr) {
            NodePtr next = curr->next;
            delete curr;
            curr = next;
        }
    }

    bool contains(std::string_view key) { return map.contains(key); }
    std::optional<std::reference_wrapper<const Node>>
    get(std::string_view key) {
        if (NodePtr target = at(key)) {
            pushNodeToFront(target);
            return std::cref(*target);
        }
        return std::nullopt;
    }

    template <class T>
        requires std::constructible_from<std::string, T> &&
                 std::convertible_to<T, std::string_view>
    Node &ask(T key, std::size_t bytes) {
        if (NodePtr keyNode = at(static_cast<std::string_view>(key))) {
            if (keyNode->real_size() >= bytes) { // inplace resize
                keyNode->size = std::max(keyNode->size, bytes);
                return *keyNode;
            }
            long long remaining = bytes - keyNode->real_size();
            NodePtr end = tail;
            NodePtr start = eject_n(required_nodes(remaining));
            tail = start->prev;
            tail->next = nullptr;
            start->prev = nullptr;
            start->child = end;

            if (keyNode->parent()) {
                keyNode->child->child->next = start;
            } else {
                keyNode->child = start;
            }

            keyNode->size = bytes;
            return *keyNode;
        }

        // owning resize
        NodePtr start_node = eject_n(required_nodes(bytes));
        start_node->has_data = true;
        start_node->child = start_node->next;
        if (start_node->child) {
            start_node->child->prev = nullptr;
            start_node->child->child = tail;
        }
        start_node->next = nullptr;
        start_node->size = bytes;
        tail = start_node;
        pushNodeToFront(start_node);
        start_node->key = key;
        map[static_cast<std::string>(key)] = start_node;
        return *start_node;
    }
    std::size_t required_nodes(std::size_t bytes) {
        return (bytes / buffer_size) + (bytes % buffer_size != 0);
    }

    // performs a copy
    void push(std::string_view key, std::string_view value) {
        Node &s = ask(key, value.size());
        for (std::size_t i{0}; i < value.size(); i++) {
            s[i] = value[i];
        }
    }

  private:
    NodePtr at(std::string_view key) {
        typename map_t::iterator it = map.find(key);
        if (it == map.end())
            return nullptr;
        return it->second;
    }
    // return the head node of the eviction
    NodePtr eject_n(std::size_t n) {
        if (n == 0)
            return nullptr;

        if (tail->has_data) {
            map.erase(tail->key);
            tail = tail->evict();
        }
        NodePtr curr = tail;
        while (--n) {
            curr = curr->prev;
            if (curr->has_data) {
                map.erase(curr->key);
                curr = curr->evict();
            }
        }
        return curr;
    }
    void pushNodeToFront(NodePtr ptr) {
        if (ptr == head)
            return;
        NodePtr left = ptr->prev;
        NodePtr right = ptr->next;
        if (right)
            right->prev = left;
        else
            tail = left;
        left->next = right;
        ptr->prev = NodePtr(nullptr);
        ptr->next = head;
        head->prev = ptr;
        head = ptr;
    }

    void display() {
        NodePtr curr = head;
        while (curr) {
            std::cout << '(' << (curr->has_data ? (curr->key) : "_");
            if (curr->parent()) {
                std::cout << '{';
                NodePtr child = curr->child;
                while (child) {
                    std::cout << "(" << (child->has_data ? (child->key) : "_")
                              << ") =>";
                    child = child->next;
                }
                std::cout << '}';
            }
            std::cout << ")->";
            curr = curr->next;
        }
        std::cout << '\n';
    }
};

template class ServerCache<65536>;
