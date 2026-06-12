#pragma once

#include <deque>
#include <vector>

/// Maintains a sliding window of recent values for streaming analysis.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   push        O(1)
///   getWindow   O(k) — copies k elements
///   Space       O(windowSize)
template <typename T>
class SlidingWindow {
public:
    explicit SlidingWindow(size_t windowSize)
        : maxSize_(windowSize) {}

    void push(const T& value) {
        window_.push_back(value);
        if (window_.size() > maxSize_) {
            window_.pop_front();
        }
    }

    std::vector<T> getWindow() const {
        return std::vector<T>(window_.begin(), window_.end());
    }

    size_t size() const { return window_.size(); }
    size_t maxSize() const { return maxSize_; }
    bool empty() const { return window_.empty(); }
    bool full() const { return window_.size() >= maxSize_; }

    const T& newest() const { return window_.back(); }
    const T& oldest() const { return window_.front(); }

    void clear() { window_.clear(); }

    /// Compute mean of current window.
    double mean() const {
        if (window_.empty()) return 0.0;
        double sum = 0.0;
        for (const auto& v : window_) sum += static_cast<double>(v);
        return sum / window_.size();
    }

private:
    std::deque<T> window_;
    size_t maxSize_;
};
