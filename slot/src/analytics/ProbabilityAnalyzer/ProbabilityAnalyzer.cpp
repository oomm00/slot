#include "ProbabilityAnalyzer.h"
#include "GamblerRuin.h"
#include "MarkovChain.h"
#include "ProbabilityDP.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

ProbabilityAnalyzer::ProbabilityAnalyzer()
    : effectivePayoutMultiplier_(PAYTABLE_MULTIPLIER_3 * 0.5 +
                                  PAYTABLE_MULTIPLIER_2 * 0.5),
      effectiveWinProb_(WIN_PROB) {}

strategyreport ProbabilityAnalyzer::reportForPlayer(
    const std::string& playerID,
    double bankroll,
    size_t recentSpins) {
    ProbabilityDP dp(effectiveWinProb_, effectivePayoutMultiplier_);
    double expectedReturn = dp.expectedProfitAfter(recentSpins, 1.0);

    double probProfit = dp.probabilityOfProfitAfter(recentSpins, 1.0);
    double risk = 1.0 - probProfit;

    double kellyFrac = dp.optimalBetSize(bankroll);

    double betAmount = bankroll * kellyFrac * 0.25;
    if (betAmount <= 0.0) betAmount = 1.0;

    GamblerRuin ruin(bankroll, bankroll * 2.0, effectiveWinProb_,
                     effectivePayoutMultiplier_);
    double ruinRisk = ruin.ruinProbability();

    MarkovChain mc(effectiveWinProb_ * 0.8, effectiveWinProb_ * 1.2);
    double dpScore = 1.0 - risk;
    double greedyScore = (expectedReturn > 0.0) ? 1.0 : 0.2;
    double kellyScore = kellyFrac;
    double confidence = 1.0 - ruinRisk;

    std::string strategy;
    if (expectedReturn <= 0.0) {
        strategy = "reduce_bet";
    } else if (risk < 0.3) {
        strategy = "aggressive";
    } else if (risk < 0.6) {
        strategy = "moderate";
    } else {
        strategy = "conservative";
    }

    return strategyreport(
        playerID, bankroll, strategy, betAmount,
        expectedReturn, risk,
        std::to_string(dpScore), std::to_string(greedyScore),
        std::to_string(kellyScore), confidence);
}

std::vector<strategyreport> ProbabilityAnalyzer::analyzeGameSessions(
    PlayerManager& pm,
    size_t recentSpins) {
    std::vector<strategyreport> reports;
    auto players = pm.getAllPlayers();

    for (const auto& pl : players) {
        double br = pl.getbal();
        reports.push_back(reportForPlayer(pl.getplayerid(), br, recentSpins));
    }

    std::sort(reports.begin(), reports.end(),
              [](const strategyreport& a, const strategyreport& b) {
                  return a.getexpectreturn() > b.getexpectreturn();
              });
    return reports;
}

ProbabilityAnalyzer::PerformanceReport ProbabilityAnalyzer::performanceReport(
    const BettingEngine::Stats& stats) {
    PerformanceReport pr{};

    pr.totalSpins = static_cast<size_t>(stats.totalBets);
    pr.totalPlayers = 0;
    pr.houseEdge = stats.totalWagered > 0.0
                       ? stats.houseProfit / stats.totalWagered
                       : 0.0;
    pr.overallWinRate = stats.winRate;

    MarkovChain mc(effectiveWinProb_ * 0.8, effectiveWinProb_ * 1.2);
    pr.avgStreakLength = mc.expectedLossStreak();

    GamblerRuin gr(100.0, 200.0, effectiveWinProb_, effectivePayoutMultiplier_);
    pr.avgRuinRisk = gr.ruinProbability();

    return pr;
}
