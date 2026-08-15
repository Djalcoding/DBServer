#pragma once
#include <alloca.h>
#include <array>
#include <climits>
#include <iostream>
#include <ostream>
#include <stdexcept>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <type_traits>
#include <unistd.h>
#include <vector>
template <std::size_t s> class segmented_string {
    static constexpr std::size_t segment_size = s;
    std::vector<char *> buffers;
    std::size_t length = 0;

  public:
    // this assumes that char * are the sizes of segment_size
    template <class... Buffers>
        requires(std::is_same_v<char *, Buffers> && ...)
    segmented_string(Buffers... b) {
        buffers.reserve(sizeof...(b));
        (buffers.push_back(b), ...);
    }
    template <class... Buffers>
        requires(std::is_same_v<std::array<char, segment_size>, Buffers> && ...)
    segmented_string(Buffers... b) : segmented_string(b.data()...) {}
    segmented_string() = delete;
    segmented_string(const segmented_string &) = delete;
    segmented_string &operator=(const segmented_string &) = delete;
    segmented_string(segmented_string &&moved) { *this = std::move(moved); }
    segmented_string &operator=(segmented_string &&moved) {
        this->buffers = std::move(moved.buffers);
    }

    void push(char *ptr) { buffers.push_back(ptr); }
    void pop(char *ptr) { buffers.pop_back(); }
    void regularize() { buffers.resize(1); }
    std::vector<char *> &data() { return buffers; }
    const std::vector<char *> &data() const { return buffers; }
    std::size_t size() { return length; };
    std::size_t byte_count() { return buffers.size() * segment_size; };

    char &operator[](std::size_t idx) {
        length = std::max(idx, length);
        return buffers[idx / segment_size][idx % segment_size];
    }
    ssize_t read(int fd) { // TODO : fix this
        iovec *buffers;
        size_t buffer_c =
            std::min(this->buffers.size(), static_cast<std::size_t>(IOV_MAX));
        buffers = static_cast<iovec *>(alloca(sizeof(iovec) * buffer_c));
        ssize_t output = 0;

        std::size_t byte_count = 0;
        ioctl(fd, FIONREAD, &byte_count);
        std::size_t buffer_start = 0;
        std::size_t required_buffers =
            (byte_count / segment_size) + (byte_count % segment_size != 0);
        if (required_buffers > this->buffers.size())
            throw std::overflow_error(
                "This container is too small for the packet");
        while (required_buffers != 0) {
            buffer_c =
                std::min(required_buffers, static_cast<std::size_t>(IOV_MAX));

            for (std::size_t i{0}; i < buffer_c; i++) {
                buffers[i] = {this->buffers[buffer_start], segment_size};
            }
            ssize_t o = ::readv(fd, buffers, buffer_c);
            if (o == -1)
                return -1;
            if (o == 0)
                break;

            buffer_start += buffer_c;
            required_buffers -= buffer_c;
            output += o;
            length = output;
        }
        return output;
    }
    friend std::ostream &operator<<(std::ostream &stream,
                                    const segmented_string &string) {
        ssize_t l = string.length;
        auto &data = string.data();
        std::size_t i = 0;
        while (l > 0) {
            auto k = std::string(data[i], l);
            stream << k.size() << ": " << k;
            l -= segment_size;
            i++;
        }
        return stream;
    }
};
