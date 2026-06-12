#pragma once

#include <vector>

/// Fixed-capacity ring buffer for storing recent numeric values.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   push    O(1) amortised
///   get     O(1)
///   size    O(1)
///   Space   O(capacity)
template <typename T>
class CircularBuffer {
public:
    explicit CircularBuffer(size_t capacity)
        : data_(capacity), capacity_(capacity), head_(0), size_(0) {}

    void push(const T& value) {
        data_[head_] = value;
        head_ = (head_ + 1) % capacity_;
        if (size_ < capacity_) ++size_;
    }

    const T& get(size_t index) const {
        if (index >= size_) index = size_ - 1;
        size_t pos = (head_ + capacity_ - size_ + index) % capacity_;
        return data_[pos];
    }

    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == capacity_; }

    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(size_);
        for (size_t i = 0; i < size_; ++i) {
            result.push_back(get(i));
        }
        return result;
    }

    void clear() {
        head_ = 0;
        size_ = 0;
    }

private:
    std::vector<T> data_;
    size_t capacity_;
    size_t head_;
    size_t size_;
};
