#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <vector>

#include "Bet.h"
#include "CircularBuffer.h"
#include "GameRecord.h"
#include "Player.h"
#include "PlayerManager.h"
#include "RabinKarpDetector.h"
#include "SlidingWindow.h"
#include "ZScoreDetector.h"

/// Fraud detection engine using multiple detection strategies.
///
/// ── Detection Strategies ────────────────────────────────────────────
///   1. Z-Score Analysis — flags bet amounts that deviate significantly
///      from a player's historical mean.
///   2. Win-Rate Anomaly — flags players whose win rate deviates
///      from the expected theoretical win rate (p = 0.2).
///   3. Pattern Repetition — detects repeated bet-amount sequences
///      that suggest scripted / automated play.
///   4. Velocity Check — flags unusually rapid betting (many spins
///      in a short window).
///   5. Balance Trajectory — flags players whose balance follows an
///      unrealistic trajectory (e.g. never goes down).
///   6. Collusion Detection — flags groups of players with
///      suspiciously correlated betting patterns.
///
/// ── Complexity ──────────────────────────────────────────────────────
///   analyzePlayer         O(k + p) — k = recent bets, p = pattern search.
///   analyzeGame           O(1).
///   checkCollusion        O(n^2 * w) — n = players, w = window size.
///   Space                 O(p × n) — per-player sliding windows.
class FraudDetector {
public:
    static constexpr double FRAUD_THRESHOLD = 0.5;
    static constexpr size_t WINDOW_SIZE = 50;
    static constexpr size_t MIN_WINS = 5;

    FraudDetector();

    /// Analyse a player's complete history and return a fraud score [0, 1].
    /// 0 = legitimate, 1 = almost certainly fraudulent.
    /// Also flags the player in PlayerManager if score exceeds threshold.
    double analyzePlayer(PlayerManager& pm,
                         const std::string& playerId,
                         const std::vector<Bet>& recentBets,
                         const std::vector<GameRecord>& recentGames);

    /// Analyse a single game record for instantaneous fraud signals.
    /// Returns a fraud score [0, 1] for this specific game.
    double analyzeGame(const GameRecord& game,
                       const std::vector<GameRecord>& playerHistory);

    /// Check for collusion between players by comparing betting patterns.
    /// Returns a vector of collusion alerts with player pairs and scores.
    struct CollusionAlert {
        std::string player1;
        std::string player2;
        double score;           // 0-1 collusion confidence
        std::string reason;     // Description of detected pattern
    };
    std::vector<CollusionAlert> checkCollusion(
        const std::vector<std::string>& playerIds,
        const PlayerManager& pm);

    /// Return overall fraud statistics.
    struct FraudStats {
        size_t totalPlayersScanned{};
        size_t flaggedPlayers{};
        double averageFraudScore{};
        double highestFraudScore{};
        size_t collusionAlerts{};   // New: collusion pairs detected
    };
    FraudStats getStats() const;

    /// Reset per-player tracking data.
    void resetPlayer(const std::string& playerId);
    void resetAll();

private:
    struct PlayerState {
        SlidingWindow<double> betAmounts{WINDOW_SIZE};
        SlidingWindow<double> winLossWindow{WINDOW_SIZE};
        CircularBuffer<long long> timestamps{100};  // epoch milliseconds
        int totalBets{};
        int totalWins{};
        double totalWagered{};
        double totalPayout{};
    };

    std::unordered_map<std::string, PlayerState> playerStates_;
    ZScoreDetector zScoreDetector_{3.0};  // threshold = 3 sigma
    RabinKarpDetector patternDetector_{3, 6};

    size_t totalFlagged_{};
    size_t totalScanned_{};
    double cumulativeScore_{};
    double maxScore_{};
    size_t totalCollusionAlerts_{};

    PlayerState& getOrCreateState(const std::string& playerId);

    double scoreBetZScore(const PlayerState& state, double amount);
    double scoreWinRateAnomaly(const PlayerState& state);
    double scorePatternRepetition(const std::vector<Bet>& bets);
    double scoreVelocity(const PlayerState& state);
    double scoreBalanceTrajectory(const std::vector<GameRecord>& games);

    // New: collusion detection helpers
    double scoreBetTimingSimilarity(const PlayerState& a, const PlayerState& b);
    double scoreBetAmountCorrelation(const PlayerState& a, const PlayerState& b);
    double scoreWinLossCorrelation(const PlayerState& a, const PlayerState& b);
    static long long parseTimestamp(const std::string& timestamp);
};
