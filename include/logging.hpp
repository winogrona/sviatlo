#pragma once

#include <vector>
#include <functional>
#include <cstddef>

#include <cstdint>

namespace logging {
    template<typename T>
    class CircularBuffer {
    public:
        explicit CircularBuffer(std::size_t capacity)
            : _capacity(capacity), _buffer(capacity), _start(0), size_(0) {}

        void push(const T& item) {
            _buffer[(_start + size_) % _capacity] = item;
            if (size_ < _capacity) {
                ++size_;
            } else {
                _start = (_start + 1) % _capacity;
            }
        }

        T& operator[](std::size_t index) {
            return _buffer[(_start + index) % _capacity];
        }

        const T& operator[](std::size_t index) const {
            return _buffer[(_start + index) % _capacity];
        }

        std::size_t size() const { return size_; }
        std::size_t capacity() const { return _capacity; }

        void clear() {
            _start = 0;
            size_ = 0;
        }

        // Iterate over elements in order
        void forEach(const std::function<void(const T&)>& func) const {
            for (std::size_t i = 0; i < size_; ++i) {
                func(_buffer[(_start + i) % _capacity]);
            }
        }

    private:
        std::size_t _capacity;
        std::vector<T> _buffer;
        std::size_t _start;
        std::size_t size_;
    };
}