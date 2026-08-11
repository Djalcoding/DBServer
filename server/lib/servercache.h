
#pragma once
#include <cstdlib>
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>

struct Node {
    using ptr = Node*;
    ptr        prev;
    std::string key;
    std::string value;
    ptr         next;
    Node(ptr prev, const std::string &key, const std::string &value, ptr next)
        : prev(prev), key(key), value(value), next(next) {}
    Node(const Node &node) = default;
    bool empty() const { return value == "" && key == ""; }
};
using NodePtr = Node::ptr;
template <bool reversed = false> struct LRUiterator {
    static constexpr int mult = reversed ? -1 : 1;
    NodePtr             ptr;
    bool                 operator==(const LRUiterator &other) const { return ptr == other.ptr; }
    LRUiterator         &operator+=(int m) {
        m *= mult;
        ptr = m > 0 ? ptr->next : ptr->prev;
        return *this;
    }
    LRUiterator &operator-=(int m) { return (*this += -m); }
    LRUiterator &operator++() { return (*this += 1); }
    LRUiterator &operator--() { return (*this -= 1); }
    LRUiterator  operator++(int) {
        LRUiterator copy = *this;
        ++(*this);
        return copy;
    }
    LRUiterator operator--(int) {
        LRUiterator copy = *this;
        --(*this);
        return copy;
    }

    const Node &operator*() { return *ptr; }
    const Node *operator->() { return ptr; }
};
// LRU
template <std::size_t length>
    requires(length >= 2)
class ServerCache {
    using key_t = std::string;
    using value_t = std::string;
    using const_iterator = LRUiterator<>;
    using reverse_const_iterator = LRUiterator<true>;
    NodePtr  head;
    NodePtr tail;

    std::unordered_map<key_t, NodePtr> map;

  public:
    ServerCache() {
        head = new Node(NodePtr(nullptr), "", "", NodePtr(nullptr));
        NodePtr prev = head;
        NodePtr  curr;
        for (std::size_t i{0}; i < length - 1; i++) {
            curr = new Node(NodePtr(prev), "", "", NodePtr(nullptr));
            prev->next = curr;
            prev = curr;
        }
        tail = curr;
    }

    ~ServerCache() {
        NodePtr prev = head;
        NodePtr curr = head->next;
        while (curr) {
            delete prev;
            prev = curr;
        } 
        delete prev;
    }

    bool                           contains(const key_t &key) { return map.contains(key); }
    std::optional<const value_t *> get(const key_t &key) {
        if (!contains(key))
            return std::nullopt;
        NodePtr target = map.at(key);
        pushNodeToFront(target);
        return &target->value;
    }

    const value_t *get_or_insert(const key_t &key, std::function<value_t()> func) {
        auto o = get(key);
        if (o.has_value()) {
            return o.value();
        } else {
            value_t output = std::move(func());
            push(key, output);
            return &map.at(key)->value;
        }
    }

    template <class Value> void push(const key_t &key, Value &&value) {
        if (contains(key)) {
            NodePtr target = map.at(key);
            pushNodeToFront(target);
            target->value = std::forward<Value>(value);
            return;
        }

        NodePtr            moved_node = tail;
        const std::string &tailKey = moved_node->key;
        if (contains(tailKey)) {
            map.erase(tailKey);
        }
        map[key] = moved_node;
        tail = moved_node->prev;
        tail->next = NodePtr(nullptr);
        moved_node->value = std::forward<Value>(value);
        moved_node->key = key;
        moved_node->prev = NodePtr(nullptr);
        moved_node->next = head;
        head->prev = moved_node;
        head = moved_node;
    }

    const_iterator begin() const { return LRUiterator{head}; }
    const_iterator end() const { return LRUiterator{tail}; }
    const_iterator cbegin() const { return LRUiterator{head}; }
    const_iterator cend() const { return LRUiterator{tail}; }

  private:
    void pushNodeToFront(NodePtr ptr) {
        if (ptr == head)
            return;
        NodePtr left = ptr->prev;
        NodePtr  right = ptr->next;
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
            std::cout << '(' << curr->key << ") ->";
            curr = curr->next;
        }
        std::cout << '\n';
        curr = tail;
        while (curr) {
            std::cout << '(' << curr->key << ") <=";
            curr = curr->prev;
        }
        std::cout << '\n';
    }
};

