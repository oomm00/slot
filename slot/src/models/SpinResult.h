#pragma once
#include <string>
#include <vector>
#include "BetType.h"

class Spin {
private:
    std::string spinid;
    std::vector<std::string> symbols;
    std::string winpattern;
    double multiplier;
    unsigned int seed;
    bool winspin;

public:
    Spin(const std::string& s, const std::vector<std::string>& sym,
         const std::string& winpat, double mult, unsigned int seed, bool winspin);

    std::string getspinid() const;
    std::vector<std::string> getsymbols() const;
    std::string getwinpattern() const;
    double getmultiplier() const;
    unsigned int getseed() const;
    bool iswinspin() const;

    void setspinid(const std::string& spinId);
    void setsymbols(const std::vector<std::string>& symbols);
    void setwinpattern(const std::string& pattern);
    void setmultiplier(double multiplier);
    void setseed(unsigned int seed);
    void setwinspin(bool winning);

    static bool evaluateExactPrediction(const std::vector<std::string>& result,
                                        const std::vector<std::string>& prediction);
    static bool evaluateTripleSymbol(const std::vector<std::string>& result,
                                     const std::string& symbol);
    static bool evaluatePairPrediction(const std::vector<std::string>& result,
                                       const std::string& symbol);
    static bool evaluateSymbolAppearance(const std::vector<std::string>& result,
                                         const std::string& symbol);
    static bool evaluateAnyPair(const std::vector<std::string>& result);
    static bool evaluateAnyTriple(const std::vector<std::string>& result);
};

struct SpinResult {
    bool success{};
    std::string playerId, spinId;
    std::vector<std::string> symbols;
    std::vector<std::string> prediction;
    std::string pattern;
    BetType betType{};
    double multiplier{}, betAmount{}, payout{};
    double balanceBefore{}, balanceAfter{};
    bool isWin{};
    double fraudScore{};
    std::string errorMessage;
};
