#include "Leaderboard.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

// ─── Public API ───────────────────────────────────────────────────────

void Leaderboard::update(const std::vector<player>& players) {
    players_ = players;
    rebuildAll();
}

// ─── Top-K queries ────────────────────────────────────────────────────

std::vector<player> Leaderboard::getTopPlayers(size_t n) const {
    return topN(sortedByBalance_, n);
}

std::vector<player> Leaderboard::getTopByBalance(size_t n) const {
    return topN(sortedByBalance_, n);
}

std::vector<player> Leaderboard::getTopByWinRate(size_t n) const {
    return topN(sortedByWinRate_, n);
}

std::vector<player> Leaderboard::getTopByTotalWinnings(size_t n) const {
    return topN(sortedByTotalWinnings_, n);
}

std::vector<player> Leaderboard::getTopByGamesPlayed(size_t n) const {
    return topN(sortedByGamesPlayed_, n);
}

std::vector<player> Leaderboard::getTopByBiggestWin(size_t n) const {
    return topN(sortedByBiggestWin_, n);
}

// ─── Rank queries ─────────────────────────────────────────────────────

player Leaderboard::getRankedPlayer(size_t rank) const {
    if (rank == 0 || rank > players_.size()) {
        throw std::out_of_range("Leaderboard::getRankedPlayer – invalid rank");
    }
    return players_[sortedByBalance_[rank - 1]];
}

size_t Leaderboard::getPlayerRank(const std::string& playerId) const {
    auto it = playerIndexMap_.find(playerId);
    if (it == playerIndexMap_.end()) return 0;

    // Binary search on sortedByBalance_ for the player's index
    size_t playerIdx = it->second;
    auto low = std::lower_bound(
        sortedByBalance_.begin(), sortedByBalance_.end(), playerIdx,
        [this](size_t idx, size_t target) {
            double v1 = players_[idx].getbal();
            double v2 = players_[target].getbal();
            if (v1 != v2) return v1 > v2;
            return players_[idx].getplayerid() < players_[target].getplayerid();
        }
    );
    return static_cast<size_t>(std::distance(sortedByBalance_.begin(), low) + 1);
}

double Leaderboard::getPercentile(const std::string& playerId) const {
    size_t rank = getPlayerRank(playerId);
    if (rank == 0) return -1.0;
    size_t total = players_.size();
    if (total <= 1) return 100.0;
    return (static_cast<double>(total - rank) / static_cast<double>(total - 1)) * 100.0;
}

size_t Leaderboard::size() const {
    return players_.size();
}

// ─── Private helpers ──────────────────────────────────────────────────

void Leaderboard::rebuildAll() {
    size_t n = players_.size();
    playerIndexMap_.clear();
    playerIndexMap_.reserve(n);

    for (size_t i = 0; i < n; ++i) {
        playerIndexMap_[players_[i].getplayerid()] = i;
    }

    auto makeIndex = [n]() {
        std::vector<size_t> idx(n);
        for (size_t i = 0; i < n; ++i) idx[i] = i;
        return idx;
    };

    // ── By balance ────────────────────────────────────────────────
    sortedByBalance_ = makeIndex();
    std::sort(sortedByBalance_.begin(), sortedByBalance_.end(),
        [this](size_t a, size_t b) {
            double va = players_[a].getbal();
            double vb = players_[b].getbal();
            if (va != vb) return va > vb;
            return players_[a].getplayerid() < players_[b].getplayerid();
        });

    // ── By win rate ──────────────────────────────────────────────
    sortedByWinRate_ = makeIndex();
    std::sort(sortedByWinRate_.begin(), sortedByWinRate_.end(),
        [this](size_t a, size_t b) {
            double va = players_[a].getwinrate();
            double vb = players_[b].getwinrate();
            if (va != vb) return va > vb;
            int ga = players_[a].getgamesplayed();
            int gb = players_[b].getgamesplayed();
            if (ga != gb) return ga > gb; // more games → higher rank on tie
            return players_[a].getplayerid() < players_[b].getplayerid();
        });

    // ── By total winnings (twon) ──────────────────────────────────
    sortedByTotalWinnings_ = makeIndex();
    std::sort(sortedByTotalWinnings_.begin(), sortedByTotalWinnings_.end(),
        [this](size_t a, size_t b) {
            double va = players_[a].gettwon();
            double vb = players_[b].gettwon();
            if (va != vb) return va > vb;
            return players_[a].getplayerid() < players_[b].getplayerid();
        });

    // ── By games played ──────────────────────────────────────────
    sortedByGamesPlayed_ = makeIndex();
    std::sort(sortedByGamesPlayed_.begin(), sortedByGamesPlayed_.end(),
        [this](size_t a, size_t b) {
            int va = players_[a].getgamesplayed();
            int vb = players_[b].getgamesplayed();
            if (va != vb) return va > vb;
            return players_[a].getplayerid() < players_[b].getplayerid();
        });

    // ── By biggest win ───────────────────────────────────────────
    sortedByBiggestWin_ = makeIndex();
    std::sort(sortedByBiggestWin_.begin(), sortedByBiggestWin_.end(),
        [this](size_t a, size_t b) {
            double va = players_[a].getbwin();
            double vb = players_[b].getbwin();
            if (va != vb) return va > vb;
            return players_[a].getplayerid() < players_[b].getplayerid();
        });
}

std::vector<player> Leaderboard::topN(const std::vector<size_t>& sortedIdx,
                                       size_t n) const {
    size_t count = std::min(n, sortedIdx.size());
    std::vector<player> result;
    result.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        result.push_back(players_[sortedIdx[i]]);
    }
    return result;
}
