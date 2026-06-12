#include "SearchEngine.h"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

#include "BinarySearch.h"
#include "KMP.h"
#include "RabinKarp.h"

// ─── Construction ────────────────────────────────────────────────────

SearchEngine::SearchEngine(PlayerManager& pm, GameLogger& gl,
                           BettingEngine& be)
    : pm_(pm), gl_(gl), be_(be) {}

// ─── Index Maintenance ───────────────────────────────────────────────

void SearchEngine::ensureIndices() {
    if (indicesDirty_) {
        buildPlayerIndex();
        buildGameIndex();
        buildBetIndex();
        indicesDirty_ = false;
    }
}

void SearchEngine::rebuildIndices() {
    indicesDirty_ = true;
    ensureIndices();
}

void SearchEngine::buildPlayerIndex() {
    auto all = pm_.getAllPlayers();
    sortedPlayerIds_.clear();
    sortedPlayerIds_.reserve(all.size());
    for (const auto& p : all) {
        sortedPlayerIds_.push_back(p.getplayerid());
    }
    std::sort(sortedPlayerIds_.begin(), sortedPlayerIds_.end());
}

void SearchEngine::buildGameIndex() {
    const auto& all = gl_.getAll();
    sortedGameIds_.clear();
    sortedGameIds_.reserve(all.size());
    for (const auto& g : all) {
        sortedGameIds_.push_back(g.getgameid());
    }
    std::sort(sortedGameIds_.begin(), sortedGameIds_.end());
}

void SearchEngine::buildBetIndex() {
    // Collect all bets from all players.
    auto allPlayers = pm_.getAllPlayers();
    sortedBets_.clear();
    sortedBetIds_.clear();
    for (const auto& p : allPlayers) {
        auto bets = be_.getPlayerBets(p.getplayerid());
        for (const auto& b : bets) {
            sortedBets_.push_back(b);
            sortedBetIds_.push_back(b.getbetid());
        }
    }
    // Sort by bet ID for binary search.
    std::vector<size_t> idx(sortedBetIds_.size());
    for (size_t i = 0; i < idx.size(); ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(),
              [this](size_t a, size_t b) {
                  return sortedBetIds_[a] < sortedBetIds_[b];
              });
    auto oldBets = sortedBets_;
    auto oldIds  = sortedBetIds_;
    for (size_t i = 0; i < idx.size(); ++i) {
        sortedBets_[i]  = oldBets[idx[i]];
        sortedBetIds_[i] = oldIds[idx[i]];
    }
}

// ─── Player Search ───────────────────────────────────────────────────

const player* SearchEngine::searchPlayerById(const std::string& playerId) {
    // Prefer O(1) hash lookup from PlayerManager; also demonstrate
    // BinarySearch for DAA documentation purposes.
    ensureIndices();
    BinarySearch<std::string> bs;
    int pos = bs.search(sortedPlayerIds_.data(),
                        sortedPlayerIds_.data() + sortedPlayerIds_.size(),
                        playerId);
    if (pos == -1) return nullptr;
    return pm_.getPlayer(sortedPlayerIds_[pos]);
}

std::vector<player> SearchEngine::searchPlayerByName(
    const std::string& namePrefix) {
    std::vector<player> result;
    auto all = pm_.getAllPlayers();
    for (const auto& p : all) {
        std::string pname = p.getname();
        // Prefix match
        if (pname.size() >= namePrefix.size() &&
            pname.compare(0, namePrefix.size(), namePrefix) == 0) {
            result.push_back(p);
        }
    }
    return result;
}

// ─── Game Search ─────────────────────────────────────────────────────

GameRecord SearchEngine::searchGameById(int gameId) {
    ensureIndices();
    BinarySearch<int> bs;
    int pos = bs.search(sortedGameIds_.data(),
                        sortedGameIds_.data() + sortedGameIds_.size(),
                        gameId);
    if (pos == -1) {
        throw std::out_of_range("Game ID not found: " + std::to_string(gameId));
    }
    // Linear scan to find the matching GameRecord.
    const auto& all = gl_.getAll();
    for (const auto& g : all) {
        if (g.getgameid() == gameId) return g;
    }
    throw std::out_of_range("Game ID not found: " + std::to_string(gameId));
}

std::vector<GameRecord> SearchEngine::searchGamesByDateRange(
    const std::string& startDate, const std::string& endDate) {
    return gl_.getByTimeRange(startDate, endDate);
}

// ─── Bet Search ──────────────────────────────────────────────────────

Bet SearchEngine::searchBetById(const std::string& betId) {
    ensureIndices();
    BinarySearch<std::string> bs;
    int pos = bs.search(sortedBetIds_.data(),
                        sortedBetIds_.data() + sortedBetIds_.size(),
                        betId);
    if (pos == -1) {
        throw std::out_of_range("Bet ID not found: " + betId);
    }
    return sortedBets_[pos];
}

// ─── Pattern Search ──────────────────────────────────────────────────

std::vector<GameRecord> SearchEngine::searchSymbolPattern(
    const std::vector<std::string>& pattern) {
    std::vector<GameRecord> matches;
    if (pattern.empty()) return matches;

    // Build concatenated symbol sequence from all game records.
    const auto& all = gl_.getAll();
    std::vector<std::string> concat;
    std::vector<size_t> gameBoundary;  // index in concat where each game starts
    gameBoundary.reserve(all.size() + 1);
    concat.reserve(all.size() * 3);
    for (const auto& g : all) {
        gameBoundary.push_back(concat.size());
        auto syms = g.getspinsymbols();
        for (const auto& s : syms) {
            concat.push_back(s);
        }
    }
    gameBoundary.push_back(concat.size());

    if (concat.size() < pattern.size()) return matches;

    // KMP search on the concatenated symbol sequence.
    KMP<std::vector<std::string>> kmp;
    auto positions = kmp.searchAll(concat, pattern);

    // Map positions back to game records.
    for (int pos : positions) {
        // Find which game this position belongs to.
        for (size_t gi = 0; gi + 1 < gameBoundary.size(); ++gi) {
            if (static_cast<size_t>(pos) >= gameBoundary[gi] &&
                static_cast<size_t>(pos) < gameBoundary[gi + 1]) {
                matches.push_back(all[gi]);
                break;
            }
        }
    }

    // Deduplicate (same game may match multiple times).
    std::sort(matches.begin(), matches.end(),
              [](const GameRecord& a, const GameRecord& b) {
                  return a.getgameid() < b.getgameid();
              });
    matches.erase(
        std::unique(matches.begin(), matches.end(),
                    [](const GameRecord& a, const GameRecord& b) {
                        return a.getgameid() == b.getgameid();
                    }),
        matches.end());

    return matches;
}

std::vector<std::vector<Bet>> SearchEngine::searchBetPattern(
    const std::vector<double>& amountPattern) {
    std::vector<std::vector<Bet>> matches;
    if (amountPattern.empty()) return matches;

    auto allPlayers = pm_.getAllPlayers();

    for (const auto& p : allPlayers) {
        auto bets = be_.getPlayerBets(p.getplayerid());
        if (bets.size() < amountPattern.size()) continue;

        // Extract amounts.
        std::vector<double> amounts;
        amounts.reserve(bets.size());
        for (const auto& b : bets) {
            amounts.push_back(b.getbetamt());
        }

        // Rabin-Karp search.
        RabinKarp<std::vector<double>> rk;
        auto positions = rk.searchAll(amounts, amountPattern);

        for (int pos : positions) {
            std::vector<Bet> seq;
            for (size_t k = 0; k < amountPattern.size(); ++k) {
                seq.push_back(bets[static_cast<size_t>(pos) + k]);
            }
            matches.push_back(std::move(seq));
        }
    }

    return matches;
}
