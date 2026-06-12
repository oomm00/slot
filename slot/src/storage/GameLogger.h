#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include "GameRecord.h"

class GameLogger {
public:
    /// Add a completed game record to the log.
    /// Updates all indices for subsequent lookups.
    void logGame(const GameRecord& rec);

    /// Retrieve all records for a given player ID.
    /// Uses hash-based lookup (O(1) average).
    std::vector<GameRecord> getPlayerHistory(const std::string& playerId) const;

    /// Retrieve the most recent `n` records across all players.
    std::vector<GameRecord> getRecentGames(size_t n) const;

    /// Get a const reference to the entire record collection.
    const std::vector<GameRecord>& getAll() const;

    /// Number of records currently stored.
    size_t size() const;

    /// Save all records to a CSV file.
    bool saveToCSV(const std::string& filepath) const;

    /// Load records from a CSV file and append them.
    /// Returns the number of records loaded, or 0 on failure.
    size_t loadFromCSV(const std::string& filepath);

    // ── DAA-enhanced lookups ──────────────────────────────────────────

    /// Binary search by game ID on a sorted index.
    /// Returns the record index in records_, or -1 if not found.
    int findGameById(int gameId) const;

    /// Time range query using binary search on sorted timestamps.
    /// Timestamps are compared lexicographically (ISO 8601 format).
    /// Returns all records with timestamp in [start, end].
    std::vector<GameRecord> getByTimeRange(const std::string& start,
                                           const std::string& end) const;

private:
    std::vector<GameRecord> records_;
    std::unordered_map<std::string, std::vector<int>> playerIndex_;

    // Sorted index of record indices by game ID — enables binary search.
    std::vector<int> sortedByGameId_;

    // Sorted index of record indices by timestamp — enables range queries.
    std::vector<int> sortedByTimestamp_;
};
