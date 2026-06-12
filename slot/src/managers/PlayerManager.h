#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "CSVStorage.h"
#include "Player.h"

/// Manages all player operations: registration, balance, persistence.
///
/// Data structures:
///   - std::unordered_map<std::string, player> for O(1) average lookup.
///
/// Complexity:
///   - addPlayer / removePlayer / getPlayer / deposit / withdraw / updateBalance:
///     O(1) average, O(n) worst-case (hash collision).
///   - getAllPlayers: O(n) to copy all entries into a vector.
///   - loadPlayers / savePlayers: O(n) for CSV I/O, O(n) to populate map.
///
/// Hash-based lookup was chosen because every player operation starts with
/// a player-ID lookup.  An unordered_map provides O(1) average find/insert/erase,
/// which dominates over tree-based (O(log n)) or list-based (O(n)) alternatives
/// given the expected thousands of players.
///
/// Space complexity: O(n) for the map, O(n) for CSV output during save.
class PlayerManager {
public:
    /// Construct with a reference to the persistence layer.
    /// The CSVStorage can be configured with a custom data directory.
    explicit PlayerManager(CSVStorage storage = CSVStorage());

    /// Register a new player.  Returns false if the player ID already exists.
    bool addPlayer(const player& p);

    /// Remove a player by ID.  Returns true if the player was found and removed.
    bool removePlayer(const std::string& playerId);

    /// Retrieve a mutable pointer to a player, or nullptr if not found.
    /// The pointer is valid until the player is removed or the manager is destroyed.
    player* getPlayer(const std::string& playerId);

    /// Deposit `amount` into the player's balance.  Amount must be positive.
    bool deposit(const std::string& playerId, double amount);

    /// Withdraw `amount` from the player's balance.  Amount must be positive
    /// and the balance must be sufficient.
    bool withdraw(const std::string& playerId, double amount);

    /// Set the player's balance to an exact value.  Must be non-negative.
    bool updateBalance(const std::string& playerId, double balance);

    /// Return a copy of all registered players.
    std::vector<player> getAllPlayers() const;

    /// Load all players from CSV into memory (replaces current state).
    /// Returns true if the file was read (even if empty).
    bool loadPlayers();

    /// Save all in-memory players to CSV.
    bool savePlayers();

private:
    std::unordered_map<std::string, player> players_;
    CSVStorage storage_;
};
