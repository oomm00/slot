#include "GameLogger.h"

#include <algorithm>
#include <fstream>
#include <sstream>
#include "BetType.h"

void GameLogger::logGame(const GameRecord& rec) {
    int index = static_cast<int>(records_.size());
    records_.push_back(rec);
    playerIndex_[rec.getplayerid()].push_back(index);

    {
        int gid = rec.getgameid();
        auto it = std::lower_bound(
            sortedByGameId_.begin(), sortedByGameId_.end(), gid,
            [this](int idx, int gid) { return records_[idx].getgameid() < gid; });
        sortedByGameId_.insert(it, index);
    }

    {
        const auto& ts = rec.gettime();
        auto it = std::lower_bound(
            sortedByTimestamp_.begin(), sortedByTimestamp_.end(), ts,
            [this](int idx, const std::string& ts) { return records_[idx].gettime() < ts; });
        sortedByTimestamp_.insert(it, index);
    }
}

std::vector<GameRecord> GameLogger::getPlayerHistory(const std::string& playerId) const {
    std::vector<GameRecord> result;
    auto it = playerIndex_.find(playerId);
    if (it == playerIndex_.end()) return result;
    result.reserve(it->second.size());
    for (int i : it->second) result.push_back(records_[i]);
    return result;
}

std::vector<GameRecord> GameLogger::getRecentGames(size_t n) const {
    std::vector<GameRecord> result;
    size_t count = std::min(n, records_.size());
    result.reserve(count);
    for (size_t i = records_.size() - count; i < records_.size(); ++i)
        result.push_back(records_[i]);
    return result;
}

bool GameLogger::saveToCSV(const std::string& filepath) const {
    std::ofstream out(filepath);
    if (!out.is_open()) return false;
    out << "gameid,playerid,betid,spinid,timestamp,betamt,bettype,"
           "prediction,spinsymbols,win,payout,balbefore,balafter,fraud\n";
    for (const auto& rec : records_) {
        auto pred = rec.getprediction();
        std::string predStr;
        for (size_t i = 0; i < pred.size(); ++i) {
            if (i > 0) predStr += ';';
            predStr += pred[i];
        }
        auto syms = rec.getspinsymbols();
        std::string symStr;
        for (size_t i = 0; i < syms.size(); ++i) {
            if (i > 0) symStr += ';';
            symStr += syms[i];
        }
        out << rec.getgameid() << ','
            << rec.getplayerid() << ','
            << rec.getbetid() << ','
            << rec.getspinid() << ','
            << rec.gettime() << ','
            << rec.getbetamt() << ','
            << BetTypeToString(rec.getbettype()) << ','
            << predStr << ','
            << symStr << ','
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
        while (std::getline(ss, field, ',')) fields.push_back(field);
        if (fields.size() < 14) continue;
        try {
            int gid = std::stoi(fields[0]);
            double amt = std::stod(fields[5]);
            BetType bt = BetTypeFromString(fields[6]);
            std::vector<std::string> prediction;
            if (!fields[7].empty()) {
                std::stringstream pss(fields[7]);
                std::string p;
                while (std::getline(pss, p, ';')) prediction.push_back(p);
            }
            std::vector<std::string> symbols;
            if (!fields[8].empty()) {
                std::stringstream sss(fields[8]);
                std::string s;
                while (std::getline(sss, s, ';')) symbols.push_back(s);
            }
            bool win = (fields[9] == "1");
            double payout = std::stod(fields[10]);
            double before = std::stod(fields[11]);
            double after = std::stod(fields[12]);
            double fraud = std::stod(fields[13]);
            GameRecord rec(gid, fields[1], fields[2], fields[3], fields[4],
                           amt, bt, prediction, symbols, win, payout,
                           before, after, fraud);
            logGame(rec);
            ++loaded;
        } catch (...) { continue; }
    }
    return loaded;
}

const std::vector<GameRecord>& GameLogger::getAll() const { return records_; }
size_t GameLogger::size() const { return records_.size(); }

int GameLogger::findGameById(int gameId) const {
    auto it = std::lower_bound(
        sortedByGameId_.begin(), sortedByGameId_.end(), gameId,
        [this](int idx, int gid) { return records_[idx].getgameid() < gid; });
    if (it != sortedByGameId_.end() && records_[*it].getgameid() == gameId)
        return *it;
    return -1;
}

std::vector<GameRecord> GameLogger::getByTimeRange(const std::string& start,
                                                    const std::string& end) const {
    auto low = std::lower_bound(
        sortedByTimestamp_.begin(), sortedByTimestamp_.end(), start,
        [this](int idx, const std::string& ts) { return records_[idx].gettime() < ts; });
    auto high = std::upper_bound(
        sortedByTimestamp_.begin(), sortedByTimestamp_.end(), end,
        [this](const std::string& ts, int idx) { return ts < records_[idx].gettime(); });
    std::vector<GameRecord> result;
    result.reserve(std::distance(low, high));
    for (auto it = low; it != high; ++it) result.push_back(records_[*it]);
    return result;
}
