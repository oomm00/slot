#pragma once

#include <string>
#include <vector>

/// Knuth-Morris-Pratt (KMP) string/sequence matching algorithm.
///
/// ── Algorithm ───────────────────────────────────────────────────────
/// 1. Preprocess the pattern to build a "failure function" (longest
///    proper prefix that is also a suffix) — the `lps` array.
/// 2. Scan the text left-to-right.  When a mismatch occurs at
///    position j in the pattern, use lps[j-1] to skip ahead without
///    re-examining previously matched characters.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   Time:  O(n + m) where n = text length, m = pattern length.
///          Every character is examined at most twice (once when
///          matching, once when falling back via LPS).
///   Space: O(m) for the LPS array.
///
/// ── DAA Justification ──────────────────────────────────────────────
/// KMP is optimal for linear-time pattern matching because it avoids
/// backtracking on the text.  The LPS array encodes the knowledge
/// gained from previous comparisons, ensuring each character is
/// compared at most O(1) times on average.
///
/// ── Alternative Comparison ─────────────────────────────────────────
///   Naive / brute force:  O(n × m) worst case.
///   Rabin-Karp:           O(n + m) average, O(n × m) worst (hash
///                         collisions).  Better for multiple patterns.
///   Boyer-Moore:          O(n + m) average, often sub-linear in
///                         practice for large alphabets, but requires
///                         more preprocessing.
///
/// KMP is chosen here for symbol-sequence matching because:
///   - The alphabet is small (5 symbols), making BM less effective.
///   - Worst-case O(n + m) is guaranteed (unlike Rabin-Karp).
///   - The LPS construction is straightforward.
template <typename T = std::string>
class KMP {
public:
    using value_type = typename T::value_type;

    /// Build LPS array for a pattern stored in a container.
    /// `pattern` is a sequence of elements (char, string, etc.).
    std::vector<int> buildLPS(const T& pattern) const {
        std::vector<int> lps(pattern.size(), 0);
        int len = 0;
        size_t i = 1;

        while (i < pattern.size()) {
            if (pattern[i] == pattern[len]) {
                ++len;
                lps[i] = len;
                ++i;
            } else {
                if (len != 0) {
                    len = lps[len - 1];
                } else {
                    lps[i] = 0;
                    ++i;
                }
            }
        }
        return lps;
    }

    /// Search for `pattern` in `text`.  Returns the starting index
    /// of the first match, or -1 if not found.
    int search(const T& text, const T& pattern) const {
        if (pattern.empty()) return 0;
        if (text.size() < pattern.size()) return -1;

        auto lps = buildLPS(pattern);
        int i = 0;  // index for text
        int j = 0;  // index for pattern

        while (i < static_cast<int>(text.size())) {
            if (text[i] == pattern[j]) {
                ++i;
                ++j;
            }

            if (j == static_cast<int>(pattern.size())) {
                return i - j;
            } else if (i < static_cast<int>(text.size()) &&
                       text[i] != pattern[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    ++i;
                }
            }
        }
        return -1;
    }

    /// Find all occurrences of `pattern` in `text`.
    std::vector<int> searchAll(const T& text, const T& pattern) const {
        std::vector<int> matches;
        if (pattern.empty()) {
            matches.push_back(0);
            return matches;
        }
        if (text.size() < pattern.size()) return matches;

        auto lps = buildLPS(pattern);
        int i = 0;
        int j = 0;

        while (i < static_cast<int>(text.size())) {
            if (text[i] == pattern[j]) {
                ++i;
                ++j;
            }

            if (j == static_cast<int>(pattern.size())) {
                matches.push_back(i - j);
                j = lps[j - 1];
            } else if (i < static_cast<int>(text.size()) &&
                       text[i] != pattern[j]) {
                if (j != 0) {
                    j = lps[j - 1];
                } else {
                    ++i;
                }
            }
        }
        return matches;
    }
};
