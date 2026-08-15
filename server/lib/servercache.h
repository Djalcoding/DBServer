
#pragma once
#include "segmented_string.h"
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

// LRU
template <std::size_t bs>
    requires(bs >= 1)
class ServerCache {
    static constexpr std::size_t storage_size = bs;
    struct Node {
        using ptr = Node *;
        bool has_data = false;
        std::size_t size = 0;
        ptr next;
        ptr prev;
        ptr child;
        std::string key;
        segmented_string<storage_size> reader;
        char data[storage_size];
        Node(ptr prev, const std::string &key, ptr next)
            : reader(data), prev(prev), key(key), next(next) {}
        bool parent() const { return child != nullptr; };

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
    };
    using NodePtr = Node::ptr;
    static constexpr std::size_t buffer_size = bs;
    using key_t = std::string;
    using value_t = std::string;
    NodePtr head;
    NodePtr tail;

    std::unordered_map<key_t, NodePtr> map;

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

    bool contains(const key_t &key) { return map.contains(key); }
    std::optional<const segmented_string<buffer_size> *> get(const key_t &key) {
        if (!contains(key))
            return std::nullopt;
        NodePtr target = map.at(key);
        pushNodeToFront(target);
        return &target->reader;
    }

    segmented_string<buffer_size>& ask(const key_t &key, std::size_t bytes) {
        std::size_t required_nodes =
            (bytes / buffer_size) + (bytes % buffer_size != 0);
        std::vector<char *> ptrs{required_nodes};
        if (tail->has_data) {
            map.erase(tail->key);
            tail = tail->evict();
        }
        NodePtr curr = tail;
        ptrs[required_nodes-1] = curr->data;
        while (--required_nodes) {
            ptrs[required_nodes-1] = curr->data;
            curr = curr->prev;
            if (curr->has_data) {
                map.erase(curr->key);
                curr = curr->evict();
            }
        }
        curr->has_data = true;
        curr->child = curr->next;
        if (curr->child) {
            curr->child->prev = nullptr;
            curr->child->child = tail;
        }
        curr->next = nullptr;
        curr->size = bytes;
        tail = curr;
        pushNodeToFront(curr);
        curr->key = key;
        map[key] = curr;
        curr->reader.data() = std::move(ptrs);
        return curr->reader;
    }

    void push(const key_t &key, std::string_view value) {
        segmented_string<buffer_size> &s = ask(key, value.size());
        for(std::size_t i{0}; i < value.size(); i++) {
            s[i] = value[i];
        }
    }

  private:
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
  public: // TODO : remove piblc
    void display() {
        NodePtr curr = head;
        while (curr) {
            std::cout << '(' << (curr->has_data ? (curr->key) : "_");
            if(curr->parent()) {
                std::cout << '{';
                NodePtr child = curr->child;
                while(child) {
                    std::cout << "(" << (child->has_data ? (child->key) : "_") << ") =>";
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
