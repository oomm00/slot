#include "SlotMachine.h"
#include <algorithm>
#include <chrono>
#include <sstream>

const std::vector<std::string> SlotMachine::DEFAULT_SYMBOLS{
    "CHERRY", "LEMON", "ORANGE", "BELL", "SEVEN", "DIAMOND"};

const std::unordered_map<std::string, SlotMachine::PayEntry>
    SlotMachine::PAYTABLE_{
        {"CHERRY", {2.0, 0.5}},
        {"LEMON",  {3.0, 1.0}},
        {"ORANGE", {5.0, 1.5}},
        {"BELL",   {7.0, 2.0}},
        {"SEVEN",  {10.0, 3.0}},
        {"DIAMOND",{50.0, 15.0}},
    };

SlotMachine::SlotMachine() : spinCounter_(0) {
    auto w = WeightedReel::defaultWeights();
    for (int i = 0; i < 3; ++i) reels_.emplace_back(w);
}

SlotMachine::SlotMachine(const std::vector<std::pair<std::string, int>>& weightedSymbols)
    : spinCounter_(0) {
    auto w = weightedSymbols.empty() ? WeightedReel::defaultWeights() : weightedSymbols;
    for (int i = 0; i < 3; ++i) reels_.emplace_back(w);
}

Spin SlotMachine::spin() { RNG rng; return spin(rng.getSeed()); }

Spin SlotMachine::spin(unsigned seed) {
    RNG rng(seed);
    std::vector<std::string> symbols;
    symbols.reserve(3);
    for (auto& reel : reels_) symbols.push_back(reel.spin(rng));

    double mult = calculateMultiplier(symbols);
    bool win = mult > 0.0;
    std::string pattern = buildPattern(symbols);
    std::string spinId = "SPIN-" + std::to_string(++spinCounter_) + "-" + std::to_string(seed);
    return Spin(spinId, symbols, pattern, mult, seed, win);
}

double SlotMachine::calculateMultiplier(const std::vector<std::string>& symbols) {
    if (symbols.size() != 3) return 0.0;
    int matches = countMatches(symbols);
    if (matches < 2) return 0.0;
    std::string sym = (matches == 3) ? findMatchSymbol(symbols, 3) : findMatchSymbol(symbols, 2);
    auto it = PAYTABLE_.find(sym);
    if (it == PAYTABLE_.end()) return 0.0;
    return (matches == 3) ? it->second.three : it->second.two;
}

double SlotMachine::evaluateBet(BetType betType,
                                 const std::vector<std::string>& result,
                                 const std::vector<std::string>& prediction) {
    bool won = false;
    if (betType == BetType::EXACT_PREDICTION) {
        won = Spin::evaluateExactPrediction(result, prediction);
    } else if (betType == BetType::TRIPLE_SYMBOL) {
        won = !prediction.empty() && Spin::evaluateTripleSymbol(result, prediction[0]);
    } else if (betType == BetType::PAIR_PREDICTION) {
        won = !prediction.empty() && Spin::evaluatePairPrediction(result, prediction[0]);
    } else if (betType == BetType::SYMBOL_APPEARANCE) {
        won = !prediction.empty() && Spin::evaluateSymbolAppearance(result, prediction[0]);
    } else if (betType == BetType::ANY_PAIR) {
        won = Spin::evaluateAnyPair(result);
    } else if (betType == BetType::ANY_TRIPLE) {
        won = Spin::evaluateAnyTriple(result);
    }
    return won ? PayoutConfig::getMultiplier(betType) : 0.0;
}

void SlotMachine::setSymbols(const std::vector<std::string>& symbols) {
    auto pool = symbols.empty() ? DEFAULT_SYMBOLS : symbols;
    reels_.clear();
    std::vector<std::pair<std::string, int>> weighted;
    for (auto& s : pool) weighted.push_back({s, 1});
    for (int i = 0; i < 3; ++i) reels_.emplace_back(weighted);
}

const std::vector<std::string>& SlotMachine::getSymbols() const {
    return reels_.empty() ? DEFAULT_SYMBOLS : reels_[0].getSymbols();
}

std::unordered_map<std::string, double> SlotMachine::getSymbolProbabilities() const {
    if (reels_.empty()) return {};
    return reels_[0].getAllProbabilities();
}

double SlotMachine::getSymbolProbability(const std::string& symbol) const {
    if (reels_.empty()) return 0.0;
    return reels_[0].getProbability(symbol);
}

int SlotMachine::countMatches(const std::vector<std::string>& syms) {
    if (syms.size() < 3) return 0;
    if (syms[0] == syms[1] && syms[1] == syms[2]) return 3;
    if (syms[0] == syms[1] || syms[0] == syms[2] || syms[1] == syms[2]) return 2;
    return 1;
}

std::string SlotMachine::findMatchSymbol(const std::vector<std::string>& syms, int matchCount) {
    if (matchCount == 3) return syms[0];
    if (syms[0] == syms[1]) return syms[0];
    if (syms[0] == syms[2]) return syms[0];
    return syms[1];
}

std::string SlotMachine::buildPattern(const std::vector<std::string>& syms) {
    if (syms.empty()) return "";
    std::string p = syms[0];
    for (size_t i = 1; i < syms.size(); ++i) p += "-" + syms[i];
    return p;
}
