#pragma once

#include <string>
#include <vector>

/// Two-state Markov Chain modelling slot-machine win/loss streaks.
///
/// States:
///   0 = LOSS (spin resulted in no payout)
///   1 = WIN  (spin resulted in a payout)
///
/// Transition matrix P:
///           LOSS    WIN
///   LOSS  [ 1-a     a  ]
///   WIN   [ 1-b     b  ]
///
/// where a = P(WIN | LOSS), b = P(WIN | WIN).
///
/// ── Computed Quantities ─────────────────────────────────────────────
///   steady-state probabilities (π₀, π₁) via eigen-decomposition:
///     π₀ = (1-b) / (2 - a - b)
///     π₁ = a / (2 - a - b)
///
///   Expected streak length of consecutive LOSSes = 1 / a
///   Expected streak length of consecutive WINs   = 1 / (1-b)
///
/// ── Complexity ──────────────────────────────────────────────────────
///   All operations O(1).
///   powerIteration(n) O(n × numStates²) = O(n × 4) = O(n).
class MarkovChain {
public:
    /// @param probWinAfterLoss  P(WIN | previous spin was LOSS)
    /// @param probWinAfterWin   P(WIN | previous spin was WIN)
    MarkovChain(double probWinAfterLoss, double probWinAfterWin);

    /// Steady-state probability of being in the LOSS state (π₀).
    double steadyStateLoss() const;

    /// Steady-state probability of being in the WIN state (π₁).
    double steadyStateWin() const;

    /// Expected consecutive losses before a win.
    double expectedLossStreak() const;

    /// Expected consecutive wins before a loss.
    double expectedWinStreak() const;

    /// Approximate steady-state via power iteration (for verification).
    /// Returns (π₀, π₁) after `iterations` steps.
    std::pair<double, double> powerIteration(size_t iterations) const;

    /// Probability of being in a given state after `steps` from a start
    /// state. Returns (prob(LOSS), prob(WIN)).
    std::pair<double, double> stateAfter(size_t steps,
                                         bool startWin) const;

private:
    double a_;  // P(WIN | LOSS)
    double b_;  // P(WIN | WIN)
};
