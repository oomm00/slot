#include "BettingAdvisor.h"
#include "BetType.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

BettingAdvisor::BettingAdvisor() {}

void BettingAdvisor::setSymbolProbabilities(const std::unordered_map<std::string, double>& probs) {
    symProbs_ = probs;
}

double BettingAdvisor::getSymProb(const std::string& sym) const {
    auto it = symProbs_.find(sym);
    return (it != symProbs_.end()) ? it->second : 0.0;
}

double BettingAdvisor::calcWinProbExact(const std::string& sym) const {
    double p = getSymProb(sym);
    return p * p * p;
}

double BettingAdvisor::calcWinProbTriple(const std::string& sym) const {
    double p = getSymProb(sym);
    return p * p * p;
}

double BettingAdvisor::calcWinProbPair(const std::string& sym) const {
    double p = getSymProb(sym);
    return 3.0 * p * p * (1.0 - p) + p * p * p;
}

double BettingAdvisor::calcWinProbAppearance(const std::string& sym) const {
    double p = getSymProb(sym);
    return 1.0 - std::pow(1.0 - p, 3);
}

double BettingAdvisor::calcWinProbAnyPair() const {
    double sum = 0.0;
    for (auto& [sym, p] : symProbs_) {
        sum += 3.0 * p * p * (1.0 - p) + p * p * p;
    }
    return sum;
}

double BettingAdvisor::calcWinProbAnyTriple() const {
    double sum = 0.0;
    for (auto& [sym, p] : symProbs_) {
        sum += p * p * p;
    }
    return sum;
}

// Quick bust probability via 1D DP (same as StrategyLab but returns just the final number)
double BettingAdvisor::computeBustProbApprox(
    double winProb, double mult,
    double balance, double betAmount, size_t numRounds) const {

    double pWin = winProb;
    double pLose = 1.0 - pWin;
    size_t M = 401;
    size_t bustIdx = M - 1;
    size_t initIdx = std::min(static_cast<size_t>(balance / 25.0), M - 2);

    std::vector<double> dp(M, 0.0);
    dp[initIdx] = 1.0;

    for (size_t rd = 0; rd < numRounds; ++rd) {
        std::vector<double> next(M, 0.0);
        for (size_t b = 0; b < M - 1; ++b) {
            if (dp[b] < 1e-15) continue;
            double bal = static_cast<double>(b) * 25.0;

            if (bal < betAmount) {
                next[bustIdx] += dp[b];
            } else {
                size_t lossIdx = static_cast<size_t>((bal - betAmount) / 25.0);
                lossIdx = std::min(lossIdx, M - 2);
                next[lossIdx] += dp[b] * pLose;
            }

            double winBal = bal + betAmount * mult;
            size_t winIdx = static_cast<size_t>(winBal / 25.0);
            winIdx = std::min(winIdx, M - 2);
            next[winIdx] += dp[b] * pWin;
        }
        next[bustIdx] += dp[bustIdx];
        dp = next;
    }

    return dp[bustIdx];
}

std::vector<AdvisorCandidate> BettingAdvisor::computeAllCandidates(
    double balance, double betAmount, size_t numRounds) {

    std::vector<AdvisorCandidate> candidates;

    auto addCandidate = [&](const std::string& label, const std::string& bt, const std::string& sym,
                            double wp, double mult) {
        double ev = wp * mult;
        double var = mult * mult * wp * (1.0 - wp);
        double bust = computeBustProbApprox(wp, mult, balance, betAmount, numRounds);
        candidates.push_back({label, bt, sym, wp, mult, ev, var, bust, 0.0});
    };

    // Non-symbol-specific bet types
    double apWP = calcWinProbAnyPair();
    addCandidate("ANY_PAIR", "ANY_PAIR", "", apWP, PayoutConfig::getMultiplier(BetType::ANY_PAIR));

    double atWP = calcWinProbAnyTriple();
    addCandidate("ANY_TRIPLE", "ANY_TRIPLE", "", atWP, PayoutConfig::getMultiplier(BetType::ANY_TRIPLE));

    // Per-symbol bet types
    for (auto& [sym, p] : symProbs_) {
        addCandidate("EXACT(" + sym + ")", "EXACT_PREDICTION", sym,
                     calcWinProbExact(sym), PayoutConfig::getMultiplier(BetType::EXACT_PREDICTION));
        addCandidate("TRIPLE(" + sym + ")", "TRIPLE_SYMBOL", sym,
                     calcWinProbTriple(sym), PayoutConfig::getMultiplier(BetType::TRIPLE_SYMBOL));
        addCandidate("PAIR(" + sym + ")", "PAIR_PREDICTION", sym,
                     calcWinProbPair(sym), PayoutConfig::getMultiplier(BetType::PAIR_PREDICTION));
        addCandidate("APPEAR(" + sym + ")", "SYMBOL_APPEARANCE", sym,
                     calcWinProbAppearance(sym), PayoutConfig::getMultiplier(BetType::SYMBOL_APPEARANCE));
    }

    return candidates;
}

RiskProfile BettingAdvisor::computeRiskProfile(
    const std::vector<Bet>& recentBets, double currentBalance) {

    RiskProfile profile;
    profile.lambda = 0.5;
    profile.recentWinRate = 0.5;
    profile.streak = 0;
    profile.balanceTrend = 0.0;

    if (recentBets.empty()) {
        profile.behaviorLabel = "New Player";
        profile.description = "No bet history — using moderate risk aversion.";
        profile.lambda = 0.5;
        return profile;
    }

    size_t K = std::min(recentBets.size(), size_t{50});
    auto start = recentBets.end() - K;

    int wins = 0, losses = 0;
    double balanceChanges = 0.0;
    int currentStreak = 0;
    std::string lastResult;
    double initialBal = currentBalance;

    for (auto it = start; it != recentBets.end(); ++it) {
        auto& bet = *it;
        bool won = bet.getrisk() > 0;
        if (won) {
            wins++;
            if (lastResult == "win") currentStreak++;
            else { currentStreak = 1; lastResult = "win"; }
        } else {
            losses++;
            if (lastResult == "loss") currentStreak--;
            else { currentStreak = -1; lastResult = "loss"; }
        }
        balanceChanges += bet.getbetamt() * (won ? bet.getrecamt() : -1.0);
    }

    profile.recentWinRate = K > 0 ? static_cast<double>(wins) / K : 0.5;
    profile.streak = currentStreak;
    profile.balanceTrend = K > 0 ? balanceChanges / K : 0.0;

    double baseLambda = 0.5;
    double streakFactor = 0.0;
    double trendFactor = 0.0;

    // Losing streak penalty — steer toward safer bets
    if (currentStreak < -2) {
        streakFactor = std::min(0.4, std::abs(currentStreak) * 0.08);
    }
    // Winning streak — slightly more aggressive, but don't over-penalize
    if (currentStreak > 3) {
        streakFactor = -0.15;
    }

    // Negative balance trend = higher risk aversion
    if (profile.balanceTrend < -currentBalance * 0.05) {
        trendFactor = 0.3;
    } else if (profile.balanceTrend < 0) {
        trendFactor = 0.1;
    }

    // Low recent win rate = higher risk aversion
    double winRateFactor = (0.5 - profile.recentWinRate) * 0.6;

    profile.lambda = std::clamp(baseLambda + streakFactor + trendFactor + winRateFactor, 0.1, 1.5);

    // Generate behavior label and description
    std::stringstream ss;
    if (currentStreak >= 5) {
        profile.behaviorLabel = "Hot Streak";
        ss << "You've won " << currentStreak << " in a row. ";
        ss << "Your recent win rate (" << (profile.recentWinRate * 100) << "%) ";
        ss << "is strong — small adjustments recommended.";
    } else if (currentStreak <= -5) {
        profile.behaviorLabel = "Chasing Losses";
        ss << "You've lost " << std::abs(currentStreak) << " in a row. ";
        ss << "Consider lower-risk bets to stabilize your balance.";
    } else if (profile.balanceTrend < 0 && profile.recentWinRate < 0.3) {
        profile.behaviorLabel = "Conservative";
        ss << "Balance is trending down (avg -" << std::abs(profile.balanceTrend) << " per bet). ";
        ss << "Prioritizing safety over high returns.";
    } else if (profile.recentWinRate > 0.6 && profile.balanceTrend > 0) {
        profile.behaviorLabel = "Aggressive";
        ss << "Strong recent performance with " << (profile.recentWinRate * 100) << "% win rate. ";
        ss << "Room for calculated risk.";
    } else {
        profile.behaviorLabel = "Balanced";
        ss << "Recent win rate of " << (profile.recentWinRate * 100) << "% ";
        ss << "with " << (currentStreak >= 0 ? "+" : "") << currentStreak << "-round streak. ";
        ss << "Using moderate strategy.";
    }
    profile.description = ss.str();

    return profile;
}

std::vector<AdvisorCandidate> BettingAdvisor::scoreAndRank(
    const std::vector<AdvisorCandidate>& candidates, double lambda) {

    std::vector<AdvisorCandidate> scored = candidates;
    for (auto& c : scored) {
        c.score = c.expectedValue - lambda * c.bustProb;
    }

    std::sort(scored.begin(), scored.end(),
        [](const auto& a, const auto& b) { return a.score > b.score; });

    return scored;
}

std::string BettingAdvisor::generateReason(
    const AdvisorCandidate& best,
    const AdvisorCandidate& second,
    const RiskProfile& profile,
    double margin) {

    std::stringstream ss;

    ss << best.label << " is the top pick ";
    ss << "(score " << best.score << ", EV " << best.expectedValue << "x, ";
    ss << (best.bustProb * 100) << "% bust risk). ";

    if (profile.lambda > 0.8) {
        ss << "Your recent pattern (" << profile.behaviorLabel;
        ss << ", \u03BB=" << profile.lambda << ") favors lower-risk bets";
        if (best.bustProb < 0.1) {
            ss << " — this option has a low bust probability.";
        } else {
            ss << ", though this still carries notable risk.";
        }
    } else if (profile.lambda < 0.3) {
        ss << "Your recent performance (" << profile.behaviorLabel;
        ss << ") supports higher-variance plays";
        if (best.expectedValue > 1.5) {
            ss << " — this option's EV is well above breakeven.";
        } else {
            ss << ", though returns may be modest.";
        }
    } else {
        ss << "Given your balanced profile, this offers the best risk-adjusted return.";
    }

    if (margin < 0.05 && margin > 0.0) {
        ss << " " << second.label << " is very close (margin " << (margin * 100) << "%) ";
        ss << "— consider splitting credits between both.";
    }

    return ss.str();
}

AdvisorRecommendation BettingAdvisor::getRecommendation(
    const std::vector<Bet>& playerHistory,
    double currentBalance,
    double betAmount,
    size_t numRounds,
    double lambdaOverride) {

    AdvisorRecommendation rec;

    auto candidates = computeAllCandidates(currentBalance, betAmount, numRounds);
    rec.riskProfile = computeRiskProfile(playerHistory, currentBalance);

    if (lambdaOverride >= 0.0) {
        rec.riskProfile.lambda = lambdaOverride;
        rec.usedPlayerHistory = false;
    } else {
        rec.usedPlayerHistory = !playerHistory.empty();
    }

    rec.rankedCandidates = scoreAndRank(candidates, rec.riskProfile.lambda);

    if (rec.rankedCandidates.empty()) {
        rec.topPick = {};
        rec.reason = "No viable bets available.";
        return rec;
    }

    rec.topPick = rec.rankedCandidates[0];

    AdvisorCandidate second;
    if (rec.rankedCandidates.size() > 1) {
        second = rec.rankedCandidates[1];
    } else {
        second = rec.rankedCandidates[0];
    }

    double margin = (rec.rankedCandidates.size() > 1)
        ? rec.rankedCandidates[0].score - rec.rankedCandidates[1].score
        : 1.0;

    rec.reason = generateReason(rec.topPick, second, rec.riskProfile, margin);

    return rec;
}
