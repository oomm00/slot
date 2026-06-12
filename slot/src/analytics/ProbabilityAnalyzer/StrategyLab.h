#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "BetType.h"

struct SymbolOdds {
    std::string name;
    double probability;
    int weight;
};

struct DistributionResult {
    std::vector<double> expectedBalances;
    std::vector<double> bustProbabilities;
    std::vector<double> stdDeviations;
    double overallBustProb;
    double overallSurvivalProb;
    double finalExpectedBalance;
};

struct BetTypeOdds {
    std::string betType;
    double winProb;
    double payoutMultiplier;
    double expectedValue;
    double variance;
    double riskScore;
    double bustProb;
    std::string riskLevel;
};

struct StrategyAllocation {
    std::string betType;
    double fraction;
    double credits;
};

struct StrategyResult {
    std::string name;
    std::vector<StrategyAllocation> allocations;
    double expectedValue;
    double expectedBalance;
    double variance;
    double bustProbability;
    double riskScore;
};

class StrategyLab {
public:
    StrategyLab();

    void setSymbolProbabilities(const std::unordered_map<std::string, double>& probs);

    std::vector<BetTypeOdds> computeAllBetOdds(double balance, double betAmount, size_t numRounds);

    BetTypeOdds computeBetOdds(BetType type, double balance, double betAmount, size_t numRounds);
    BetTypeOdds computeBetOddsForSymbol(BetType type, const std::string& symbol,
                                         double balance, double betAmount, size_t numRounds);

    DistributionResult computeDistribution(double winProb, double mult,
                                            double balance, double betAmount, size_t numRounds);
    DistributionResult computeDistributionForType(BetType type, double balance,
                                                   double betAmount, size_t numRounds,
                                                   const std::string& symbol = "");

    // Strategy engines
    StrategyResult greedyStrategy(double balance, double betAmount, size_t numRounds);
    StrategyResult dpStrategy(double balance, double betAmount, size_t numRounds,
                              double riskTolerance = 0.15);
    StrategyResult kellyStrategy(double balance, double betAmount, size_t numRounds);

    static std::vector<StrategyResult> compareAllStrategies(
        double balance, double betAmount, size_t numRounds, double riskTolerance = 0.15);

    // Per-symbol per-type EV table
    std::vector<std::vector<double>> computeEvTable(double balance, double betAmount, size_t numRounds);

    static std::vector<BetType> allPredictionTypes();

private:
    std::unordered_map<std::string, double> symProbs_;
    double getSymProb(const std::string& sym) const;

    double calcWinProbExact(const std::string& sym) const;
    double calcWinProbTriple(const std::string& sym) const;
    double calcWinProbPair(const std::string& sym) const;
    double calcWinProbAppearance(const std::string& sym) const;
    double calcWinProbAnyPair() const;
    double calcWinProbAnyTriple() const;

    static std::string riskLevel(double risk);
};
