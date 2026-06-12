#include "MarkovChain.h"

#include <cmath>
#include <utility>

MarkovChain::MarkovChain(double probWinAfterLoss, double probWinAfterWin)
    : a_(probWinAfterLoss), b_(probWinAfterWin) {}

double MarkovChain::steadyStateLoss() const {
    double denom = 1.0 - b_ + a_;
    if (denom == 0.0) return 0.5;
    return (1.0 - b_) / denom;
}

double MarkovChain::steadyStateWin() const {
    double denom = 1.0 - b_ + a_;
    if (denom == 0.0) return 0.5;
    return a_ / denom;
}

double MarkovChain::expectedLossStreak() const {
    if (a_ <= 0.0) return std::numeric_limits<double>::infinity();
    return 1.0 / a_;
}

double MarkovChain::expectedWinStreak() const {
    if (b_ >= 1.0) return std::numeric_limits<double>::infinity();
    return 1.0 / (1.0 - b_);
}

std::pair<double, double> MarkovChain::stateAfter(size_t steps,
                                                   bool startWin) const {
    double pLoss = startWin ? (1.0 - b_) : (1.0 - a_);
    double pWin  = startWin ? b_ : a_;

    for (size_t i = 1; i < steps; ++i) {
        double nextLoss = pLoss * (1.0 - a_) + pWin * (1.0 - b_);
        double nextWin  = pLoss * a_ + pWin * b_;
        pLoss = nextLoss;
        pWin  = nextWin;
    }
    return {pLoss, pWin};
}

std::pair<double, double> MarkovChain::powerIteration(
    size_t iterations) const {
    double piLoss = 0.5, piWin = 0.5;

    for (size_t i = 0; i < iterations; ++i) {
        double nextLoss = piLoss * (1.0 - a_) + piWin * (1.0 - b_);
        double nextWin  = piLoss * a_ + piWin * b_;
        piLoss = nextLoss;
        piWin  = nextWin;
    }
    return {piLoss, piWin};
}
