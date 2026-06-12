#pragma once
#include <string>
#include <unordered_map>
#include <vector>

enum class BetType {
    EXACT_PREDICTION = 0,
    TRIPLE_SYMBOL = 1,
    PAIR_PREDICTION = 2,
    SYMBOL_APPEARANCE = 3,
    ANY_PAIR = 4,
    ANY_TRIPLE = 5
};

struct PayoutConfig {
    static const std::unordered_map<BetType, double>& multipliers() {
        static const std::unordered_map<BetType, double> m = {
            {BetType::EXACT_PREDICTION,   50.0},
            {BetType::TRIPLE_SYMBOL,      30.0},
            {BetType::PAIR_PREDICTION,     8.0},
            {BetType::SYMBOL_APPEARANCE,   3.0},
            {BetType::ANY_PAIR,           10.0},
            {BetType::ANY_TRIPLE,         25.0}
        };
        return m;
    }
    static double getMultiplier(BetType t) {
        auto it = multipliers().find(t);
        return it != multipliers().end() ? it->second : 1.0;
    }
};

inline std::string BetTypeToString(BetType t) {
    switch (t) {
        case BetType::EXACT_PREDICTION:  return "EXACT";
        case BetType::TRIPLE_SYMBOL:     return "TRIPLE_SYMBOL";
        case BetType::PAIR_PREDICTION:   return "PAIR";
        case BetType::SYMBOL_APPEARANCE: return "SYMBOL_APPEARANCE";
        case BetType::ANY_PAIR:          return "ANY_PAIR";
        case BetType::ANY_TRIPLE:        return "ANY_TRIPLE";
    }
    return "UNKNOWN";
}

inline BetType BetTypeFromString(const std::string& s) {
    if (s == "EXACT" || s == "EXACT_PREDICTION") return BetType::EXACT_PREDICTION;
    if (s == "TRIPLE_SYMBOL")  return BetType::TRIPLE_SYMBOL;
    if (s == "PAIR" || s == "PAIR_PREDICTION") return BetType::PAIR_PREDICTION;
    if (s == "SYMBOL_APPEARANCE") return BetType::SYMBOL_APPEARANCE;
    if (s == "ANY_PAIR")      return BetType::ANY_PAIR;
    if (s == "ANY_TRIPLE")    return BetType::ANY_TRIPLE;
    return BetType::EXACT_PREDICTION;
}

inline std::string getBetTypeDescription(BetType t) {
    switch (t) {
        case BetType::EXACT_PREDICTION:  return "Predict all 3 symbols in exact order - 50x payout";
        case BetType::TRIPLE_SYMBOL:     return "Predict a specific symbol appears on all 3 reels - 30x payout";
        case BetType::PAIR_PREDICTION:   return "Predict a specific symbol appears at least twice - 8x payout";
        case BetType::SYMBOL_APPEARANCE: return "Predict a specific symbol appears at least once - 3x payout";
        case BetType::ANY_PAIR:          return "Any symbol appears at least twice - 10x payout";
        case BetType::ANY_TRIPLE:        return "All 3 symbols match - 25x payout";
    }
    return "Unknown bet type";
}

inline std::vector<BetType> allBetTypes() {
    return {
        BetType::EXACT_PREDICTION,
        BetType::TRIPLE_SYMBOL,
        BetType::PAIR_PREDICTION,
        BetType::SYMBOL_APPEARANCE,
        BetType::ANY_PAIR,
        BetType::ANY_TRIPLE
    };
}
