#pragma once

#include <algorithm>
#include <functional>
#include <string>
#include <vector>

/// Generic binary search on a sorted random-access range.
///
/// ── Algorithm ───────────────────────────────────────────────────────
/// Repeatedly divides the search interval in half by comparing the
/// target against the middle element.  If the middle matches, return
/// the index.  Otherwise, discard the half that cannot contain the
/// target and continue on the remaining half.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   Time:  O(log n) comparisons in the worst case.
///   Space: O(1) — iterative, no recursion stack.
///
/// ── DAA Justification ──────────────────────────────────────────────
/// Binary search is optimal for comparison-based search on sorted data
/// because each comparison eliminates half of the remaining candidates,
/// achieving the information-theoretic lower bound of Ω(log n).
///
/// ── Alternative Comparison ─────────────────────────────────────────
///   Linear search:  O(n) — better for unsorted data or very small n.
///   Hash lookup:    O(1) avg — faster but requires a hash function
///                   and does not support range / prefix queries.
///   BST / std::set: O(log n) — same asymptotic cost but higher
///                   constant factor and cache overhead.
///
/// Binary search is chosen here because it operates in-place on a
/// sorted vector (no extra data structure), has optimal cache
/// behaviour (sequential access), and seamlessly supports prefix
/// matching via lower_bound on the prefix range.
template <typename T, typename Compare = std::less<T>>
class BinarySearch {
public:
    explicit BinarySearch(Compare comp = Compare{})
        : comp_(std::move(comp)) {}

    /// Find the exact index of `target` in the sorted range [begin, end).
    /// Returns the index, or -1 if not found.
    int search(const T* begin, const T* end, const T& target) const {
        const T* it = std::lower_bound(begin, end, target, comp_);
        if (it != end && !(comp_(target, *it)) && !(comp_(*it, target))) {
            return static_cast<int>(it - begin);
        }
        return -1;
    }

    /// Find all elements that have `prefix` as a string prefix.
    /// The range must be sorted lexicographically.
    std::vector<std::reference_wrapper<const T>> prefixSearch(
        const T* begin, const T* end, const T& prefix) const {
        std::vector<std::reference_wrapper<const T>> result;
        auto lo = std::lower_bound(begin, end, prefix, comp_);
        // Upper bound: first element that does NOT start with prefix.
        // We find it by searching for the next lexicographic string.
        T next = prefix;
        nextAppend(next);
        auto hi = std::lower_bound(begin, end, next, comp_);

        for (auto it = lo; it != hi; ++it) {
            result.push_back(std::cref(*it));
        }
        return result;
    }

    /// Lower bound: first index ≥ target.
    ptrdiff_t lowerBound(const T* begin, const T* end,
                         const T& target) const {
        return std::lower_bound(begin, end, target, comp_) - begin;
    }

private:
    Compare comp_;

    /// Mutate `s` to the smallest string lexicographically greater
    /// than any string starting with the original `s`.
    static void nextAppend(std::string& s) {
        if (s.empty()) { s.push_back('\0'); return; }
        s.back() = static_cast<char>(s.back() + 1);
    }

    // For non-string types, no-op overload
    template <typename U>
    static void nextAppend(U& val) {
        val = val + 1;
    }
};

/// Specialised integer binary search with no append concept.
template <typename T>
class BinarySearch<T, std::less<T>> {
public:
    explicit BinarySearch(std::less<T> = {}) {}

    int search(const T* begin, const T* end, const T& target) const {
        auto it = std::lower_bound(begin, end, target);
        if (it != end && target == *it) {
            return static_cast<int>(it - begin);
        }
        return -1;
    }

    ptrdiff_t lowerBound(const T* begin, const T* end,
                         const T& target) const {
        return std::lower_bound(begin, end, target) - begin;
    }
};
