#include "RabinKarpDetector.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_map>

#include "RabinKarp.h"

int RabinKarpDetector::countRepeats(
    const std::vector<double>& amounts,
    const std::vector<double>& pattern) const {
    if (amounts.size() < pattern.size() || pattern.empty()) return 0;

    RabinKarp<std::vector<double>> rk;
    auto matches = rk.searchAll(amounts, pattern);

    // Deduplicate overlapping matches.
    int count = 0;
    int lastEnd = -1;
    // matches is sorted ascending
    for (int pos : matches) {
        if (pos >= lastEnd) {
            ++count;
            lastEnd = pos + static_cast<int>(pattern.size());
        }
    }
    return count;
}

std::vector<double> RabinKarpDetector::findMostRepeated(
    const std::vector<double>& amounts) const {
    if (amounts.size() < minPatternLen_) return {};

    int bestCount = 0;
    std::vector<double> bestPattern;

    for (size_t len = minPatternLen_;
         len <= std::min(maxPatternLen_, amounts.size()); ++len) {
        std::map<std::vector<double>, int> patternCounts;

        for (size_t start = 0; start + len <= amounts.size(); ++start) {
            std::vector<double> pat(amounts.begin() + start,
                                    amounts.begin() + start + len);

            // Only count non-overlapping occurrences.
            if (patternCounts.find(pat) == patternCounts.end()) {
                int cnt = countRepeats(amounts, pat);
                patternCounts[pat] = cnt;
            }
        }

        for (const auto& kv : patternCounts) {
            if (kv.second > bestCount) {
                bestCount = kv.second;
                bestPattern = kv.first;
            }
        }
    }

    return bestPattern;
}

double RabinKarpDetector::repetitionScore(
    const std::vector<double>& amounts) const {
    if (amounts.size() < minPatternLen_ * 2) return 0.0;

    auto best = findMostRepeated(amounts);
    if (best.empty()) return 0.0;

    int repeats = countRepeats(amounts, best);
    if (repeats <= 1) return 0.0;

    // Score based on how many times the pattern repeats relative to
    // the total number of bets.
    double totalPatternElements = static_cast<double>(repeats) * best.size();
    double ratio = totalPatternElements / amounts.size();

    return std::min(1.0, ratio);
}

bool RabinKarpDetector::matches(const std::vector<double>& data,
                                 size_t start,
                                 const std::vector<double>& pattern) const {
    if (start + pattern.size() > data.size()) return false;
    for (size_t i = 0; i < pattern.size(); ++i) {
        if (std::abs(data[start + i] - pattern[i]) > 0.001) return false;
    }
    return true;
}
