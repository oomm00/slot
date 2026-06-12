#pragma once

#include <string>
#include <vector>

/// Rabin-Karp string/sequence matching using a rolling hash.
///
/// ── Algorithm ───────────────────────────────────────────────────────
/// 1. Compute a hash of the pattern.
/// 2. Compute the hash of each length-m window of the text using a
///    rolling hash (add new char, remove old char in O(1)).
/// 3. When the window hash matches the pattern hash, verify
///    character-by-character to rule out spurious collisions.
///
/// Uses a polynomial rolling hash:
///   hash(s) = sum(s[i] * base^(m-1-i)) mod mod
///
/// ── Complexity ──────────────────────────────────────────────────────
///   Time:  O(n + m) average case.
///          O(n × m) worst case (many hash collisions).
///   Space: O(1) — only a few integers.
///
/// ── DAA Justification ──────────────────────────────────────────────
/// Rabin-Karp is chosen for betting-pattern matching because:
///   - Multiple patterns can be matched simultaneously (one hash
///     per pattern, compare all in O(1) per window).
///   - The rolling hash avoids O(m) per window, making it faster
///     than KMP for short patterns over large texts.
///   - Good average-case behaviour for numerical sequences.
///
/// ── Alternative Comparison ─────────────────────────────────────────
///   KMP:              O(n + m) guaranteed, single pattern only.
///   Boyer-Moore:      O(n + m) average, can be sub-linear.
///   Brute force:      O(n × m) worst case.
///   Aho-Corasick:     O(n + Σm) — best for many patterns.
///
/// Rabin-Karp is used here for bet-amount patterns because:
///   - Bet amounts are double values, well-suited to hashing.
///   - We may search for multiple patterns concurrently.
///   - The rolling window avoids redundant computation.
template <typename T = std::string>
class RabinKarp {
public:
    using value_type = typename T::value_type;

    static constexpr long long BASE = 131;
    static constexpr long long MOD = 1'000'000'007;

    /// Compute rolling hash for the entire text, then find first
    /// occurrence of `pattern`.
    int search(const T& text, const T& pattern) const {
        if (pattern.empty()) return 0;
        if (text.size() < pattern.size()) return -1;

        long long patHash = hash(pattern, 0, pattern.size());
        long long txtHash = hash(text, 0, pattern.size());
        long long power = modPow(BASE, pattern.size() - 1);

        if (patHash == txtHash && verify(text, pattern, 0)) return 0;

        for (size_t i = pattern.size(); i < text.size(); ++i) {
            txtHash = roll(txtHash, text[i - pattern.size()], text[i], power);
            if (txtHash == patHash && verify(text, pattern, i - pattern.size() + 1)) {
                return static_cast<int>(i - pattern.size() + 1);
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

        long long patHash = hash(pattern, 0, pattern.size());
        long long txtHash = hash(text, 0, pattern.size());
        long long power = modPow(BASE, pattern.size() - 1);

        if (patHash == txtHash && verify(text, pattern, 0)) {
            matches.push_back(0);
        }

        for (size_t i = pattern.size(); i < text.size(); ++i) {
            txtHash = roll(txtHash, text[i - pattern.size()], text[i], power);
            if (txtHash == patHash && verify(text, pattern, i - pattern.size() + 1)) {
                matches.push_back(static_cast<int>(i - pattern.size() + 1));
            }
        }
        return matches;
    }

private:
    /// Polynomial hash of a substring.
    long long hash(const T& s, size_t start, size_t len) const {
        long long h = 0;
        for (size_t i = start; i < start + len; ++i) {
            h = (h * BASE + static_cast<long long>(s[i])) % MOD;
        }
        return h;
    }

    /// Rolling hash: remove `oldChar`, add `newChar`.
    long long roll(long long oldHash, value_type oldChar,
                   value_type newChar, long long power) const {
        long long h = (oldHash - static_cast<long long>(oldChar) * power % MOD + MOD) % MOD;
        h = (h * BASE + static_cast<long long>(newChar)) % MOD;
        return h;
    }

    /// Verify character-by-character to rule out hash collision.
    bool verify(const T& text, const T& pattern, size_t start) const {
        if (start + pattern.size() > text.size()) return false;
        for (size_t i = 0; i < pattern.size(); ++i) {
            if (text[start + i] != pattern[i]) return false;
        }
        return true;
    }

    /// Modular exponentiation.
    long long modPow(long long base, long long exp) const {
        long long result = 1;
        while (exp > 0) {
            if (exp & 1) result = (result * base) % MOD;
            base = (base * base) % MOD;
            exp >>= 1;
        }
        return result;
    }
};

/// Specialisation for std::vector<double> — bet amount patterns.
template <>
class RabinKarp<std::vector<double>> {
public:
    static constexpr long long BASE = 131;
    static constexpr long long MOD = 1'000'000'007;

    int search(const std::vector<double>& text,
               const std::vector<double>& pattern) const {
        if (pattern.empty()) return 0;
        if (text.size() < pattern.size()) return -1;

        auto vecToStr = [](const std::vector<double>& v) {
            std::string s;
            s.reserve(v.size() * 8);
            for (double d : v) {
                long long bits;
                memcpy(&bits, &d, sizeof(bits));
                for (int i = 0; i < 8; ++i) {
                    s.push_back(static_cast<char>((bits >> (i * 8)) & 0xFF));
                }
            }
            return s;
        };

        std::string txtStr = vecToStr(text);
        std::string patStr = vecToStr(pattern);
        RabinKarp<std::string> rk;
        int pos = rk.search(txtStr, patStr);
        return (pos == -1) ? -1 : pos / 8;
    }

    std::vector<int> searchAll(const std::vector<double>& text,
                                const std::vector<double>& pattern) const {
        std::vector<int> matches;
        if (pattern.empty()) {
            matches.push_back(0);
            return matches;
        }
        if (text.size() < pattern.size()) return matches;

        auto vecToStr = [](const std::vector<double>& v) {
            std::string s;
            s.reserve(v.size() * 8);
            for (double d : v) {
                long long bits;
                memcpy(&bits, &d, sizeof(bits));
                for (int i = 0; i < 8; ++i) {
                    s.push_back(static_cast<char>((bits >> (i * 8)) & 0xFF));
                }
            }
            return s;
        };

        std::string txtStr = vecToStr(text);
        std::string patStr = vecToStr(pattern);
        RabinKarp<std::string> rk;
        auto raw = rk.searchAll(txtStr, patStr);
        matches.reserve(raw.size());
        for (int pos : raw) {
            if (pos % 8 == 0) {
                matches.push_back(pos / 8);
            }
        }
        return matches;
    }
};
