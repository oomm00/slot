#pragma once

#include <string>

/// Closed-form Gambler's Ruin analysis for a player making repeated bets.
///
/// Models a player starting with `initialBankroll` who bets `betAmount`
/// per spin, with probability `p` of winning (payout multiplier `b`).
/// The "ruin" point is when the player reaches 0.
///
/// ── Formulas ────────────────────────────────────────────────────────
///   Let q = 1 - p.
///
///   Fair-odds case (p == q == 0.5):
///     Ruin probability = 1 - (initial / goal)
///     Expected spins   = initial * (goal - initial)
///
///   Unfair case (p ≠ q):
///     Let r = q / p  (odds ratio, r ≠ 1)
///     Ruin probability =
///         (r^initial - r^goal) / (1 - r^goal)
///     Expected duration =
///         (initial - (goal * (1 - r^initial)) / (1 - r^goal)) / (q - p)
///
/// ── Complexity ──────────────────────────────────────────────────────
///   All operations O(1).
///   Uses `std::pow(r, n)` for exponentiation.
class GamblerRuin {
public:
    /// @param initialBankroll  Starting units (e.g. dollars).
    /// @param goalBankroll     Target units at which player stops.
    /// @param winProb          Probability of winning a single spin.
    /// @param payoutMultiplier Net payout multiplier on win (e.g. 0.98).
    GamblerRuin(double initialBankroll, double goalBankroll,
                double winProb, double payoutMultiplier);

    /// Probability of eventually going bankrupt before reaching the goal.
    double ruinProbability() const;

    /// Probability of reaching the goal before ruin.
    double successProbability() const;

    /// Expected number of spins until ruin or goal is reached.
    double expectedDuration() const;

    // -- Convenience queries --

    /// Maximum safe bet such that ruin probability stays below `threshold`.
    double maxSafeBet(double ruinThreshold = 0.05) const;

private:
    double initial_;
    double goal_;
    double p_;  // win probability
    double b_;  // payout multiplier

    double effectiveWinProb() const;
    double oddsRatio() const;
};
