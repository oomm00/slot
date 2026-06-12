#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "BettingEngine.h"
#include "GamblerRuin.h"
#include "MarkovChain.h"
#include "PlayerManager.h"
#include "ProbabilityDP.h"
#include "StrategyReport.h"

/// Integrates ProbabilityDP, MarkovChain, and GamblerRuin into a single
/// analysis facade that also accepts BettingEngine stats and produces
/// per-player strategy recommendations.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   reportForPlayer     O(N^2)  — delegates to ProbabilityDP::profitDistribution
///   analyzeGameSessions O(N^2 × P) — per-player DP analysis
///   performanceReport   O(N^2 × P) — aggregates over all players
class ProbabilityAnalyzer {
public:
    static constexpr double PAYTABLE_MULTIPLIER_3 = 5.4;
    static constexpr double PAYTABLE_MULTIPLIER_2 = 1.6;
    static constexpr double WIN_PROB = 0.2;

    ProbabilityAnalyzer();

    /// Build a DP + Markov + Ruin analysis for one player using a
    /// BettingEngine window (last N spins).
    strategyreport reportForPlayer(
        const std::string& playerID,
        double bankroll,
        size_t recentSpins);

    /// Analyze all players in a PlayerManager.
    std::vector<strategyreport> analyzeGameSessions(
        PlayerManager& pm,
        size_t recentSpins);

    /// Summarise overall game fairness metrics from global BettingEngine stats.
    struct PerformanceReport {
        double houseEdge;
        double overallWinRate;
        double avgStreakLength;
        double avgRuinRisk;
        size_t totalPlayers;
        size_t totalSpins;
    };

    PerformanceReport performanceReport(const BettingEngine::Stats& stats);

private:
    double effectivePayoutMultiplier_{};
    double effectiveWinProb_{};
};
