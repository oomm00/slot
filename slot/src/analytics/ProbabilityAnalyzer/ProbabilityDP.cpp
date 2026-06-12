#include "ProbabilityDP.h"

#include <cmath>
#include <vector>

ProbabilityDP::ProbabilityDP(double winProb, double payoutMultiplier)
    : winProb_(winProb), payoutMultiplier_(payoutMultiplier) {}

double ProbabilityDP::edgePerBet() const {
    return winProb_ * payoutMultiplier_ - 1.0;
}

double ProbabilityDP::expectedProfitAfter(size_t numBets,
                                          double betAmount) const {
    return static_cast<double>(numBets) * betAmount * edgePerBet();
}

std::vector<double> ProbabilityDP::profitDistribution(size_t numBets) const {
    std::vector<double> dp(numBets + 1, 0.0);
    dp[0] = 1.0;

    for (size_t j = 0; j < numBets; ++j) {
        for (size_t i = j + 1; i > 0; --i) {
            dp[i] = dp[i] * (1.0 - winProb_) + dp[i - 1] * winProb_;
        }
        dp[0] *= (1.0 - winProb_);
    }
    return dp;
}

double ProbabilityDP::probabilityOfProfitAfter(size_t numBets,
                                                double betAmount) const {
    auto dist = profitDistribution(numBets);
    double prob = 0.0;

    for (size_t i = 0; i <= numBets; ++i) {
        double totalBet = static_cast<double>(numBets) * betAmount;
        double totalWin = static_cast<double>(i) * payoutMultiplier_ * betAmount;
        if (totalWin > totalBet) {
            prob += dist[i];
        }
    }
    return prob;
}

double ProbabilityDP::optimalBetSize(double bankroll,
                                     double kellyFraction) const {
    double edge = edgePerBet();
    if (edge <= 0.0 || bankroll <= 0.0) return 0.0;

    double p = winProb_;
    double b = payoutMultiplier_;
    double fullKelly = (p * b - (1.0 - p)) / b;

    if (fullKelly <= 0.0) return 0.0;
    return kellyFraction * fullKelly;
}
