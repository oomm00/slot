#pragma once

#include <string>
#include <unordered_map>
#include <vector>

/// Dynamic Programming analysis for slot-machine expected outcomes.
///
/// Uses a binomial DP to compute the probability distribution of wins
/// over N independent spins, then derives expected profit, probability
/// of profit, and optimal bet sizing via the Kelly criterion.
///
/// ── Recurrence ──────────────────────────────────────────────────────
///   dp[i][j] = probability of exactly i wins in j spins
///   dp[0][0] = 1.0
///   dp[i][j] = p * dp[i-1][j-1]  +  (1-p) * dp[i][j-1]
///
///   Expected profit after N spins = N * bet * (p * avgMultiplier - 1)
///
/// ── Complexity ──────────────────────────────────────────────────────
///   expectedProfitAfter      O(1)       — closed form
///   profitDistribution       O(N^2)     — binomial DP table
///   probabilityOfProfitAfter O(N^2)     — sum over dp[i][N] where payout > bet
///   optimalBetSize           O(1)       — Kelly fraction formula
///
///   Space: O(N) or O(N^2) depending on method.
class ProbabilityDP {
public:
    /// @param winProb       Probability of a winning spin (0..1).
    /// @param payoutMultiplier  Average payout multiplier on a win.
    ProbabilityDP(double winProb, double payoutMultiplier);

    /// Expected net profit (may be negative) after `numBets` of `betAmount`.
    double expectedProfitAfter(size_t numBets, double betAmount) const;

    /// Full probability distribution of win counts.
    /// result[i] = probability of exactly i wins in numBets spins.
    std::vector<double> profitDistribution(size_t numBets) const;

    /// Probability of being in profit after `numBets` (total payout > total bet).
    double probabilityOfProfitAfter(size_t numBets, double betAmount) const;

    /// Kelly-optimal fraction of bankroll to bet.
    /// Returns fraction of bankroll (0..1), or 0 if edge <= 0.
    double optimalBetSize(double bankroll, double kellyFraction = 1.0) const;

private:
    double winProb_;
    double payoutMultiplier_;

    /// Edge per unit bet: expected return on 1 unit bet.
    double edgePerBet() const;
};
