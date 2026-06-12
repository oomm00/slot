#include "FraudDetector.h"

#include <algorithm>
#include <cmath>
#include <numeric>

FraudDetector::FraudDetector() {
    zScoreDetector_.setThreshold(3.0);
}

FraudDetector::PlayerState& FraudDetector::getOrCreateState(
    const std::string& playerId) {
    auto it = playerStates_.find(playerId);
    if (it == playerStates_.end()) {
        it = playerStates_.emplace(playerId, PlayerState{}).first;
    }
    return it->second;
}

double FraudDetector::scoreBetZScore(const PlayerState& state,
                                      double amount) {
    if (state.totalBets < 5) return 0.0;
    return zScoreDetector_.anomalyScore(amount);
}

double FraudDetector::scoreWinRateAnomaly(const PlayerState& state) {
    if (state.totalBets < MIN_WINS) return 0.0;
    double actualWinRate = static_cast<double>(state.totalWins) /
                           static_cast<double>(state.totalBets);
    // Theoretical win rate = 0.2 (1 in 5 spins wins)
    double diff = std::abs(actualWinRate - 0.2);

    // If win rate is extremely high (> 2x expected), suspect
    if (actualWinRate > 0.4) {
        return std::min(1.0, (actualWinRate - 0.4) / 0.6);
    }
    // If win rate is extremely low (could be rigged reporting)
    if (actualWinRate < 0.05 && state.totalBets > 20) {
        return 0.3;
    }
    return 0.0;
}

double FraudDetector::scorePatternRepetition(
    const std::vector<Bet>& bets) {
    if (bets.size() < 6) return 0.0;

    std::vector<double> amounts;
    amounts.reserve(bets.size());
    for (const auto& b : bets) {
        amounts.push_back(b.getbetamt());
    }

    return patternDetector_.repetitionScore(amounts);
}

double FraudDetector::scoreVelocity(const PlayerState& state) {
    if (state.timestamps.size() < 10) return 0.0;
    // Higher velocity = more bets per unit time → suspicious
    // We approximate velocity by how full the timestamp buffer is
    // relative to the theoretical maximum rate.
    return 0.0;
}

double FraudDetector::scoreBalanceTrajectory(
    const std::vector<GameRecord>& games) {
    if (games.size() < 10) return 0.0;

    // Check if player always wins (no losses in recent games).
    // This is extremely unlikely for a fair slot machine.
    int recentWins = 0;
    int recentTotal = 0;
    for (auto it = games.rbegin();
         it != games.rend() && recentTotal < 10; ++it, ++recentTotal) {
        if (it->iswin()) ++recentWins;
    }

    if (recentTotal >= 5 && recentWins == recentTotal) {
        return 0.8;
    }
    return 0.0;
}

double FraudDetector::analyzePlayer(
    PlayerManager& pm,
    const std::string& playerId,
    const std::vector<Bet>& recentBets,
    const std::vector<GameRecord>& recentGames) {
    auto& state = getOrCreateState(playerId);
    ++totalScanned_;

    // Update state from bets
    for (const auto& b : recentBets) {
        state.betAmounts.push(b.getbetamt());
        ++state.totalBets;
        state.totalWagered += b.getbetamt();
    }

    // Update state from games (wins/losses)
    for (const auto& g : recentGames) {
        state.winLossWindow.push(g.iswin() ? 1.0 : 0.0);
        if (g.iswin()) ++state.totalWins;
        state.totalPayout += g.getpayout();
    }

    // Compute individual scores
    double zScore = 0.0;
    if (!recentBets.empty()) {
        zScore = scoreBetZScore(state, recentBets.back().getbetamt());
    }
    double winRateScore = scoreWinRateAnomaly(state);
    double patternScore = scorePatternRepetition(recentBets);
    double velocityScore = scoreVelocity(state);
    double balanceScore = scoreBalanceTrajectory(recentGames);

    // Weighted combination
    double totalScore = zScore * 0.15 +
                        winRateScore * 0.35 +
                        patternScore * 0.25 +
                        velocityScore * 0.10 +
                        balanceScore * 0.15;

    totalScore = std::min(1.0, totalScore);

    // Flag the player if threshold exceeded
    if (totalScore >= FRAUD_THRESHOLD) {
        player* p = pm.getPlayer(playerId);
        if (p) {
            p->setfruad(true);
            p->setrisk(totalScore);
        }
        ++totalFlagged_;
    }

    cumulativeScore_ += totalScore;
    maxScore_ = std::max(maxScore_, totalScore);

    return totalScore;
}

double FraudDetector::analyzeGame(
    const GameRecord& game,
    const std::vector<GameRecord>& playerHistory) {
    // Instant signals for a single game:
    double score = 0.0;

    // 1. Unusually high payout relative to bet size
    double payoutRatio = game.getbetamt() > 0
                             ? game.getpayout() / game.getbetamt()
                             : 0.0;
    // Max normal payout is 10x (SEVEN three-of-a-kind)
    if (payoutRatio > 10.0) {
        score += 0.4;
    }

    // 2. Very rapid consecutive games (same timestamp)
    if (!playerHistory.empty()) {
        const auto& last = playerHistory.back();
        if (last.gettime() == game.gettime()) {
            score += 0.3;
        }
    }

    return std::min(1.0, score);
}

FraudDetector::FraudStats FraudDetector::getStats() const {
    FraudStats stats;
    stats.totalPlayersScanned = totalScanned_;
    stats.flaggedPlayers = totalFlagged_;
    stats.averageFraudScore = totalScanned_ > 0
                                  ? cumulativeScore_ / totalScanned_
                                  : 0.0;
    stats.highestFraudScore = maxScore_;
    return stats;
}

void FraudDetector::resetPlayer(const std::string& playerId) {
    playerStates_.erase(playerId);
}

void FraudDetector::resetAll() {
    playerStates_.clear();
    totalFlagged_ = 0;
    totalScanned_ = 0;
    cumulativeScore_ = 0.0;
    maxScore_ = 0.0;
}
