#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "Heap.h"
#include "Player.h"

/// Ranks players by multiple performance metrics.
///
/// ── Data structures ─────────────────────────────────────────────────
///   players_              vector<player>           primary storage
///   sortedBy*_            vector<size_t>           indices sorted by metric (desc)
///   playerIndexMap_       unordered_map<string,size_t>  O(1) id → position
///
/// ── Complexity ──────────────────────────────────────────────────────
///   update               O(n log n)  — full sort for each of 5 categories
///   getTopBy* (top-K)    O(n log k)  — heap-based top-K via heap_top_k()
///   getRankedPlayer      O(1)        — direct index into sortedByBalance_
///   getPlayerRank        O(log n)    — binary search on sortedByBalance_
///   getPercentile        O(1)        — after rank is known
///   size                 O(1)
///
/// ── DAA justification ───────────────────────────────────────────────
/// Top-K queries use **heap selection** (MinHeap / heap_top_k) instead of
/// a full sort or a sorted-vector slice.  The heap approach builds a
/// min-heap of size K in O(K), then processes the remaining N−K elements
/// in O((N−K) log K) = O(N log K), which is asymptotically optimal for
/// top-K when K ≪ N.  Rank and percentile queries still rely on fully
/// sorted index vectors (built by update()), since they need the complete
/// ordering.
///
/// Rejected alternatives:
///   1. `std::multiset` (balanced BST) — O(log n) insert but O(n) rank
///      lookup (no random-access iterator).
///   2. `std::priority_queue` (max-heap) — O(k log n) top-K extraction
///      but no support for rank / percentile queries.
///
class Leaderboard {
public:
    /// Replace all player data and rebuild every sorted index.
    void update(const std::vector<player>& players);

    // ── Top-K queries (O(n log k)) ─────────────────────────────────────

    /// Global top players (by balance).
    std::vector<player> getTopPlayers(size_t n) const;

    /// Top players by current balance.
    std::vector<player> getTopByBalance(size_t n) const;

    /// Top players by win rate (wins / games played).
    std::vector<player> getTopByWinRate(size_t n) const;

    /// Top players by total winnings (twon).
    std::vector<player> getTopByTotalWinnings(size_t n) const;

    /// Top players by number of games played.
    std::vector<player> getTopByGamesPlayed(size_t n) const;

    /// Top players by biggest single win.
    std::vector<player> getTopByBiggestWin(size_t n) const;

    // ── Rank queries ──────────────────────────────────────────────────

    /// Player at position `rank` (1-indexed) in the global ranking.
    /// Throws std::out_of_range if rank is 0 or greater than size().
    player getRankedPlayer(size_t rank) const;

    /// Global rank for a player (1 = best).  Returns 0 if the player
    /// is not in the leaderboard.
    size_t getPlayerRank(const std::string& playerId) const;

    /// Percentile (0.0–100.0) for a player.  100.0 = highest rank.
    /// Returns -1.0 if the player is not found.
    double getPercentile(const std::string& playerId) const;

    /// Number of players currently tracked.
    size_t size() const;

private:
    // ── Primary storage ────────────────────────────────────────────
    std::vector<player> players_;
    std::unordered_map<std::string, size_t> playerIndexMap_;

    // ── Sorted indices (each is a permutation of 0..n-1) ───────────
    std::vector<size_t> sortedByBalance_;
    std::vector<size_t> sortedByWinRate_;
    std::vector<size_t> sortedByTotalWinnings_;
    std::vector<size_t> sortedByGamesPlayed_;
    std::vector<size_t> sortedByBiggestWin_;

    // ── Internals ──────────────────────────────────────────────────
    void rebuildAll();
};
