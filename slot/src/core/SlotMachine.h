#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "WeightedReel.h"
#include "RNG.h"
#include "SpinResult.h"
#include "BetType.h"

class SlotMachine {
public:
    static const std::vector<std::string> DEFAULT_SYMBOLS;

    SlotMachine();
    explicit SlotMachine(const std::vector<std::pair<std::string, int>>& weightedSymbols);

    Spin spin();
    Spin spin(unsigned seed);

    static double calculateMultiplier(const std::vector<std::string>& symbols);
    static double evaluateBet(BetType betType,
                               const std::vector<std::string>& result,
                               const std::vector<std::string>& prediction);

    void setSymbols(const std::vector<std::string>& symbols);
    const std::vector<std::string>& getSymbols() const;

    std::unordered_map<std::string, double> getSymbolProbabilities() const;
    double getSymbolProbability(const std::string& symbol) const;

private:
    std::vector<WeightedReel> reels_;
    int spinCounter_;

    static int countMatches(const std::vector<std::string>& syms);
    static std::string findMatchSymbol(const std::vector<std::string>& syms, int matchCount);
    static std::string buildPattern(const std::vector<std::string>& syms);

    struct PayEntry { double three; double two; };
    static const std::unordered_map<std::string, PayEntry> PAYTABLE_;
};
