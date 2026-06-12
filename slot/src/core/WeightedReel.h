#pragma once
#include <string>
#include <vector>
#include <utility>
#include <unordered_map>
#include "RNG.h"

class WeightedReel {
public:
    WeightedReel(std::vector<std::pair<std::string, int>> symbolWeights);

    std::string spin(RNG& rng) const;
    const std::vector<std::string>& getSymbols() const;
    const std::vector<int>& getWeights() const;
    int totalWeight() const;
    double getProbability(const std::string& symbol) const;
    std::unordered_map<std::string, double> getAllProbabilities() const;

    static std::vector<std::pair<std::string, int>> defaultWeights();

private:
    std::vector<std::string> symbols_;
    std::vector<int> weights_;
    int totalWeight_;
};
