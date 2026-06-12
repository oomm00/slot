#include "WeightedReel.h"
#include <numeric>
#include <stdexcept>

WeightedReel::WeightedReel(std::vector<std::pair<std::string, int>> symbolWeights)
    : totalWeight_(0) {
    if (symbolWeights.empty()) {
        symbolWeights = defaultWeights();
    }
    for (auto& [sym, w] : symbolWeights) {
        if (w <= 0) continue;
        symbols_.push_back(sym);
        weights_.push_back(w);
        totalWeight_ += w;
    }
    if (symbols_.empty()) {
        symbols_ = {"CHERRY", "LEMON", "ORANGE", "BELL", "SEVEN"};
        weights_ = {30, 25, 20, 15, 8};
        totalWeight_ = 98;
    }
}

std::string WeightedReel::spin(RNG& rng) const {
    int roll = rng.nextInt(1, totalWeight_);
    int accum = 0;
    for (size_t i = 0; i < symbols_.size(); ++i) {
        accum += weights_[i];
        if (roll <= accum) return symbols_[i];
    }
    return symbols_.back();
}

const std::vector<std::string>& WeightedReel::getSymbols() const { return symbols_; }
const std::vector<int>& WeightedReel::getWeights() const { return weights_; }
int WeightedReel::totalWeight() const { return totalWeight_; }

double WeightedReel::getProbability(const std::string& symbol) const {
    for (size_t i = 0; i < symbols_.size(); ++i) {
        if (symbols_[i] == symbol)
            return static_cast<double>(weights_[i]) / totalWeight_;
    }
    return 0.0;
}

std::unordered_map<std::string, double> WeightedReel::getAllProbabilities() const {
    std::unordered_map<std::string, double> probs;
    for (size_t i = 0; i < symbols_.size(); ++i) {
        probs[symbols_[i]] = static_cast<double>(weights_[i]) / totalWeight_;
    }
    return probs;
}

std::vector<std::pair<std::string, int>> WeightedReel::defaultWeights() {
    return {
        {"CHERRY", 30},
        {"LEMON", 25},
        {"ORANGE", 20},
        {"BELL", 15},
        {"SEVEN", 8},
        {"DIAMOND", 2}
    };
}
