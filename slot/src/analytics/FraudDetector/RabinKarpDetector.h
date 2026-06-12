#pragma once

#include <string>
#include <vector>

/// Detects repeated patterns in bet-amount sequences using Rabin-Karp.
///
/// This is a lightweight wrapper that detects whether a player's
/// recent bet amounts exhibit suspicious repetition — e.g. the same
/// sequence of 3+ bet amounts appearing multiple times — which may
/// indicate automated / scripted play.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   search        O(n + m) avg via Rabin-Karp rolling hash.
///   findRepeats   O(n^2 + n*m) — searches all sub-patterns.
///   Space         O(m) for pattern storage.
class RabinKarpDetector {
public:
    RabinKarpDetector(size_t minPatternLen = 3, size_t maxPatternLen = 6)
        : minPatternLen_(minPatternLen), maxPatternLen_(maxPatternLen) {}

    /// Count how many times `pattern` repeats in `amounts`.
    int countRepeats(const std::vector<double>& amounts,
                     const std::vector<double>& pattern) const;

    /// Find the most frequently repeated sub-pattern within the
    /// given amounts vector.
    std::vector<double> findMostRepeated(
        const std::vector<double>& amounts) const;

    /// Compute a suspicion score [0, 1] based on pattern repetition.
    double repetitionScore(const std::vector<double>& amounts) const;

private:
    size_t minPatternLen_;
    size_t maxPatternLen_;

    bool matches(const std::vector<double>& data, size_t start,
                 const std::vector<double>& pattern) const;
};
