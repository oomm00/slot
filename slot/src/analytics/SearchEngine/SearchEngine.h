#pragma once

#include <string>
#include <vector>

#include "Bet.h"
#include "BettingEngine.h"
#include "GameLogger.h"
#include "GameRecord.h"
#include "Player.h"
#include "PlayerManager.h"

/// High-performance search system for querying players, bets, games,
/// and betting patterns using multiple DAA search algorithms.
///
/// ── Algorithms Used ─────────────────────────────────────────────────
///   searchPlayerById       Binary Search  — O(log n)
///   searchGameById         Binary Search  — O(log n)
///   searchBetById          Binary Search  — O(log n)
///   searchSymbolPattern    KMP            — O(n + m)
///   searchBetPattern       Rabin-Karp     — O(n + m) avg
///
/// ── Complexity ──────────────────────────────────────────────────────
///   Building sorted indices  O(n log n) once, then O(log n) per query.
///   Pattern matching         O(n + m) per query.
///   Date range filtering     O(n) linear scan.
class SearchEngine {
public:
    SearchEngine(PlayerManager& pm, GameLogger& gl, BettingEngine& be);

    // ── Player Search ──────────────────────────────────────────────

    /// Exact player lookup by ID using Binary Search on a sorted
    /// index of player IDs.  Returns nullptr if not found.
    const player* searchPlayerById(const std::string& playerId);

    /// Prefix-match search on player names (linear scan, unsorted).
    std::vector<player> searchPlayerByName(const std::string& namePrefix);

    // ── Game Search ────────────────────────────────────────────────

    /// Exact game lookup by game ID using Binary Search.
    /// Throws std::out_of_range if not found.
    GameRecord searchGameById(int gameId);

    /// Retrieve all games within a date range (inclusive).
    /// Delegates to GameLogger::getByTimeRange.
    std::vector<GameRecord> searchGamesByDateRange(
        const std::string& startDate, const std::string& endDate);

    // ── Bet Search ─────────────────────────────────────────────────

    /// Exact bet lookup by bet ID using Binary Search.
    /// Returns the Bet, or throws std::out_of_range if not found.
    Bet searchBetById(const std::string& betId);

    // ── Pattern Search ─────────────────────────────────────────────

    /// Search for a sequence of symbols (e.g. {"CHERRY","SEVEN"})
    /// appearing in any game record's spin symbols.
    /// Uses KMP pattern matching.
    /// Returns all game records whose spin-symbol sequence contains
    /// the given pattern as a contiguous subsequence.
    std::vector<GameRecord> searchSymbolPattern(
        const std::vector<std::string>& pattern);

    /// Search for a sequence of bet amounts appearing as consecutive
    /// bets in any player's bet history.
    /// Uses Rabin-Karp rolling hash.
    /// Returns a vector of bet-sequences (one per matching player run).
    std::vector<std::vector<Bet>> searchBetPattern(
        const std::vector<double>& amountPattern);

    // ── Maintenance ────────────────────────────────────────────────

    /// Rebuild sorted indices (e.g. after data changes).
    void rebuildIndices();

private:
    PlayerManager& pm_;
    GameLogger& gl_;
    BettingEngine& be_;

    // Sorted indices for binary search
    std::vector<std::string> sortedPlayerIds_;
    std::vector<int> sortedGameIds_;
    std::vector<std::string> sortedBetIds_;
    std::vector<Bet> sortedBets_;

    bool indicesDirty_ = true;

    void ensureIndices();
    void buildPlayerIndex();
    void buildGameIndex();
    void buildBetIndex();
};
