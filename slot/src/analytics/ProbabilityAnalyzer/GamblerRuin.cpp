#include "GamblerRuin.h"

#include <cmath>
#include <limits>

GamblerRuin::GamblerRuin(double initialBankroll, double goalBankroll,
                          double winProb, double payoutMultiplier)
    : initial_(initialBankroll),
      goal_(goalBankroll),
      p_(winProb),
      b_(payoutMultiplier) {}

double GamblerRuin::effectiveWinProb() const {
    return p_;
}

double GamblerRuin::oddsRatio() const {
    double q = 1.0 - p_;
    return q / p_;
}

double GamblerRuin::ruinProbability() const {
    if (initial_ >= goal_) return 0.0;
    if (initial_ <= 0.0) return 1.0;

    double p = effectiveWinProb();
    double q = 1.0 - p;

    if (p == q) {
        return 1.0 - (initial_ / goal_);
    }

    double r = oddsRatio();

    // For large exponents, pow(r, n) may overflow.
    // When r > 1 and goal is large: divide num/denom by r^goal
    //   ruin = (r^(I-G) - 1) / (r^(-G) - 1) → 1 - r^(I-G)
    // When r < 1 and goal is large: r^goal → 0
    //   ruin = (rI - 0) / (1 - 0) = rI

    if (r > 1.0) {
        // Compute r^(I-G) which is ≤ 1 since I < G
        double rDiff = std::pow(r, initial_ - goal_);
        return 1.0 - rDiff;
    }

    // r < 1: r^goal → 0 for large goal
    double rI = std::pow(r, initial_);
    double rG = std::pow(r, goal_);

    if (rG == 1.0) {
        return 1.0 - (initial_ / goal_);
    }

    return (rI - rG) / (1.0 - rG);
}

double GamblerRuin::successProbability() const {
    return 1.0 - ruinProbability();
}

double GamblerRuin::expectedDuration() const {
    if (initial_ >= goal_) return 0.0;
    if (initial_ <= 0.0) return 0.0;

    double p = effectiveWinProb();
    double q = 1.0 - p;
    double denom = q - p;

    if (p == q || denom == 0.0) {
        return initial_ * (goal_ - initial_);
    }

    double r = oddsRatio();

    if (r > 1.0) {
        double rIDiff = std::pow(r, initial_ - goal_);
        double numer = initial_ - goal_ * rIDiff;
        return numer / denom;
    }

    double rI = std::pow(r, initial_);
    double rG = std::pow(r, goal_);

    if (rG >= 1.0) {
        double rIDiff = std::pow(r, initial_ - goal_);
        double numer = initial_ - goal_ * rIDiff;
        return numer / denom;
    }

    double numer = initial_ - goal_ * (1.0 - rI) / (1.0 - rG);
    return numer / denom;
}

double GamblerRuin::maxSafeBet(double ruinThreshold) const {
    if (ruinThreshold <= 0.0) return 0.0;

    double targetRuin = ruinThreshold;
    double hi = initial_;
    double lo = 0.0;

    for (int iter = 0; iter < 100; ++iter) {
        double mid = (lo + hi) / 2.0;
        if (mid <= 0.0) { lo = mid; continue; }

        GamblerRuin snapshot(initial_, goal_, effectiveWinProb(), b_);
        double actualRuin = snapshot.ruinProbability();

        if (actualRuin > targetRuin)
            hi = mid;
        else
            lo = mid;
    }
    return (lo + hi) / 2.0;
}
