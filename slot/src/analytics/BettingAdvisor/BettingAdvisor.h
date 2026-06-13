#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "Bet.h"
#include "BetType.h"

struct AdvisorCandidate {
    std::string label;
    std::string betType;
    std::string symbol;
    double winProb;
    double payoutMultiplier;
    double expectedValue;
    double variance;
    double bustProb;
    double score;
};

struct RiskProfile {
    double lambda;
    double recentWinRate;
    int streak;
    double balanceTrend;
    std::string behaviorLabel;
    std::string description;
};

struct AdvisorRecommendation {
    std::vector<AdvisorCandidate> rankedCandidates;
    RiskProfile riskProfile;
    AdvisorCandidate topPick;
    std::string reason;
    bool usedPlayerHistory;
};

class BettingAdvisor {
public:
    BettingAdvisor();

    void setSymbolProbabilities(const std::unordered_map<std::string, double>& probs);

    std::vector<AdvisorCandidate> computeAllCandidates(double balance, double betAmount, size_t numRounds);

    RiskProfile computeRiskProfile(const std::vector<Bet>& recentBets, double currentBalance);

    std::vector<AdvisorCandidate> scoreAndRank(const std::vector<AdvisorCandidate>& candidates, double lambda);

    std::string generateReason(const AdvisorCandidate& best,
                                const AdvisorCandidate& second,
                                const RiskProfile& profile,
                                double margin);

    AdvisorRecommendation getRecommendation(const std::vector<Bet>& playerHistory,
                                              double currentBalance,
                                              double betAmount,
                                              size_t numRounds,
                                              double lambdaOverride = -1.0);

private:
    std::unordered_map<std::string, double> symProbs_;
    double getSymProb(const std::string& sym) const;

    double calcWinProbExact(const std::string& sym) const;
    double calcWinProbTriple(const std::string& sym) const;
    double calcWinProbPair(const std::string& sym) const;
    double calcWinProbAppearance(const std::string& sym) const;
    double calcWinProbAnyPair() const;
    double calcWinProbAnyTriple() const;

    double computeBustProbApprox(double winProb, double mult,
                                  double balance, double betAmount, size_t numRounds) const;
};
