#include "GameLogger.h"

#include <algorithm>
#include <fstream>
#include <sstream>

void GameLogger::logGame(const GameRecord& rec) {
    int index = static_cast<int>(records_.size());
    records_.push_back(rec);

    // ── Hash index (O(1) avg) ────────────────────────────────────────
    playerIndex_[rec.getplayerid()].push_back(index);

    // ── Insert into game-ID sorted index (O(n), dominated by shift) ──
    {
        int gid = rec.getgameid();
        auto it = std::lower_bound(
            sortedByGameId_.begin(), sortedByGameId_.end(), gid,
            [this](int idx, int gid) {
                return records_[idx].getgameid() < gid;
            }
        );
        sortedByGameId_.insert(it, index);
    }

    // ── Insert into timestamp sorted index (O(n)) ────────────────────
    {
        const auto& ts = rec.gettime();
        auto it = std::lower_bound(
            sortedByTimestamp_.begin(), sortedByTimestamp_.end(), ts,
            [this](int idx, const std::string& ts) {
                return records_[idx].gettime() < ts;
            }
        );
        sortedByTimestamp_.insert(it, index);
    }
}

std::vector<GameRecord> GameLogger::getPlayerHistory(const std::string& playerId) const {
    std::vector<GameRecord> result;
    auto it = playerIndex_.find(playerId);
    if (it == playerIndex_.end()) {
        return result;
    }
    result.reserve(it->second.size());
    for (int i : it->second) {
        result.push_back(records_[i]);
    }
    return result;
}

std::vector<GameRecord> GameLogger::getRecentGames(size_t n) const {
    std::vector<GameRecord> result;
    size_t count = std::min(n, records_.size());
    result.reserve(count);
    for (size_t i = records_.size() - count; i < records_.size(); ++i) {
        result.push_back(records_[i]);
    }
    return result;
}

bool GameLogger::saveToCSV(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;

    out << "gameid,playerid,betid,spinid,timestamp,betamt,bettype,spinsymbols,"
           "win,payout,balbefore,balafter,fraud\n";

    for (const auto& rec : records_) {
        out << rec.getgameid() << ','
            << rec.getplayerid() << ','
            << rec.getbetid() << ','
            << rec.getspinid() << ','
            << rec.gettime() << ','
            << rec.getbetamt() << ','
            << rec.getbettype() << ',';

        {
            const auto& syms = rec.getspinsymbols();
            for (size_t i = 0; i < syms.size(); ++i) {
                if (i > 0) out << ';';
                out << syms[i];
            }
        }

        out << ','
            << (rec.iswin() ? '1' : '0') << ','
            << rec.getpayout() << ','
            << rec.getbalbefore() << ','
            << rec.getbalafter() << ','
            << rec.getfraud() << '\n';
    }

    return out.good();
}

size_t GameLogger::loadFromCSV(const std::string& filepath) {
    std::ifstream in(filepath);
    if (!in.is_open()) return 0;

    std::string line;
    size_t loaded = 0;

    std::getline(in, line);

    while (std::getline(in, line)) {
        if (line.empty()) continue;

        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string field;

        while (std::getline(ss, field, ',')) {
            fields.push_back(field);
        }

        if (fields.size() != 13) continue;

        try {
            int gid = std::stoi(fields[0]);
            std::string pid = fields[1];
            std::string bid = fields[2];
            std::string sid = fields[3];
            std::string ts = fields[4];
            double amt = std::stod(fields[5]);
            std::string btype = fields[6];

            std::vector<std::string> symbols;
            if (!fields[7].empty()) {
                std::stringstream symss(fields[7]);
                std::string sym;
                while (std::getline(symss, sym, ';')) {
                    symbols.push_back(sym);
                }
            }

            bool win = (fields[8] == "1");
            double payout = std::stod(fields[9]);
            double before = std::stod(fields[10]);
            double after = std::stod(fields[11]);
            double fraud = std::stod(fields[12]);

            GameRecord rec(gid, pid, bid, sid, ts, amt, btype,
                           symbols, win, payout, before, after, fraud);
            logGame(rec);
            ++loaded;
        } catch (...) {
            continue;
        }
    }

    return loaded;
}

// ── DAA-enhanced lookups ─────────────────────────────────────────────

int GameLogger::findGameById(int gameId) const {
    auto it = std::lower_bound(
        sortedByGameId_.begin(), sortedByGameId_.end(), gameId,
        [this](int idx, int gid) {
            return records_[idx].getgameid() < gid;
        }
    );
    if (it != sortedByGameId_.end() && records_[*it].getgameid() == gameId) {
        return *it;
    }
    return -1;
}

std::vector<GameRecord> GameLogger::getByTimeRange(const std::string& start,
                                                    const std::string& end) const {
    auto low = std::lower_bound(
        sortedByTimestamp_.begin(), sortedByTimestamp_.end(), start,
        [this](int idx, const std::string& ts) {
            return records_[idx].gettime() < ts;
        }
    );
    auto high = std::upper_bound(
        sortedByTimestamp_.begin(), sortedByTimestamp_.end(), end,
        [this](const std::string& ts, int idx) {
            return ts < records_[idx].gettime();
        }
    );

    std::vector<GameRecord> result;
    result.reserve(std::distance(low, high));
    for (auto it = low; it != high; ++it) {
        result.push_back(records_[*it]);
    }
    return result;
}
