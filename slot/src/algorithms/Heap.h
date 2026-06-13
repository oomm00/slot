#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <vector>

/// ── Min-heap for top-K selection ──────────────────────────────────────
///
/// Instead of sorting all N elements (O(N log N)), heap-based top-K
/// builds a **min-heap of size K** and processes the remaining N−K
/// elements in O(N log K) time.
///
/// ── Algorithm ─────────────────────────────────────────────────────────
///   1. Push first K elements onto a min-heap.
///   2. For each remaining element x:
///        if comp(x, root)  →  the new element has higher priority than
///                             the current K-th best → replace root.
///   3. Extract all K elements (they come out ascending; the result is
///      reversed so the best element is first).
///
/// ── Usage ────────────────────────────────────────────────────────────
///   auto cmp = [](const player& a, const player& b) {
///       return a.getbal() > b.getbal();   // descending by balance
///   };
///   auto top = heap_top_k(players, 10, cmp);
///
template <typename T, typename Compare = std::less<T>>
class MinHeap {
public:
    explicit MinHeap(Compare comp = Compare()) : comp_(std::move(comp)) {}

    void push(const T& val) {
        data_.push_back(val);
        siftUp(data_.size() - 1);
    }

    void pop() {
        if (data_.empty()) return;
        data_[0] = data_.back();
        data_.pop_back();
        if (!data_.empty()) siftDown(0);
    }

    const T& top() const { return data_[0]; }
    size_t size() const { return data_.size(); }
    bool empty() const { return data_.empty(); }

    void build(const std::vector<T>& data) {
        data_ = data;
        for (int i = (static_cast<int>(data_.size()) / 2) - 1; i >= 0; --i) {
            siftDown(static_cast<size_t>(i));
        }
    }

private:
    std::vector<T> data_;
    Compare comp_;

    // ── Sift helpers ──────────────────────────────────────────────
    // The heap invariant: comp_(parent, child) is true for all nodes.
    // For std::less<T> this gives a min-heap (root is the "least" element).

    void siftUp(size_t i) {
        while (i > 0) {
            size_t p = (i - 1) / 2;
            if (comp_(data_[p], data_[i])) break;   // parent already ≤ child
            std::swap(data_[p], data_[i]);
            i = p;
        }
    }

    void siftDown(size_t i) {
        size_t n = data_.size();
        while (true) {
            size_t best = i;
            size_t l = 2 * i + 1;
            size_t r = 2 * i + 2;
            // The "best" is the smallest according to comp_
            if (l < n && !comp_(data_[best], data_[l])) best = l;
            if (r < n && !comp_(data_[best], data_[r])) best = r;
            if (best == i) break;
            std::swap(data_[i], data_[best]);
            i = best;
        }
    }
};

/// ── Heap-based top-K selection ────────────────────────────────────────
///
/// Returns the top `k` elements from `input` according to `comp`.
/// `comp(a, b)` should return true when `a` is **higher priority**
/// than `b` (i.e., a should appear before b in the final sorted result).
///
/// Example — top 10 players by descending balance:
///   auto top = heap_top_k(players, 10,
///       [](const player& a, const player& b) {
///           return a.getbal() > b.getbal();
///       });
///
/// Complexity: O(N log K) time, O(K) extra space.
///
template <typename T, typename Compare>
std::vector<T> heap_top_k(const std::vector<T>& input, size_t k,
                          Compare comp) {
    if (k == 0 || input.empty()) return {};
    k = std::min(k, input.size());

    // Min-heap with INVERTED comparison:
    // The comp function says "a is higher priority than b".
    // We want the heap root to be the LOWEST priority element among the
    // current top-K candidates.  So we negate: rootLowest(a, b) = comp(b, a).
    auto invert = [&comp](const T& a, const T& b) { return comp(b, a); };
    MinHeap<T, decltype(invert)> heap(invert);

    // Phase 1: prime the heap with the first k elements
    for (size_t i = 0; i < k; ++i) heap.push(input[i]);

    // Phase 2: for each remaining element, if it beats the current K-th
    //          best, replace it.
    for (size_t i = k; i < input.size(); ++i) {
        // comp(input[i], root) → input[i] outranks the current K-th best
        if (comp(input[i], heap.top())) {
            heap.pop();
            heap.push(input[i]);
        }
    }

    // Phase 3: extract all elements — they come out in ascending priority
    //          order; reverse so the best element is first.
    std::vector<T> result;
    result.reserve(k);
    for (size_t i = 0; i < k; ++i) {
        result.push_back(heap.top());
        heap.pop();
    }
    std::reverse(result.begin(), result.end());
    return result;
}
