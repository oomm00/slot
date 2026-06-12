#include "CSVStorage.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "BetType.h"

CSVStorage::CSVStorage(std::string dataDir) : dataDir_(std::move(dataDir)) {}

bool CSVStorage::openOutput(const std::string& path, std::ofstream& out) const {
    out.open(filePath(path));
    return out.is_open();
}

bool CSVStorage::openInput(const std::string& path, std::ifstream& in) const {
    in.open(filePath(path));
    return in.is_open();
}

std::string CSVStorage::filePath(const std::string& filename) const {
    if (dataDir_.empty()) return filename;
    return dataDir_ + '/' + filename;
}

void CSVStorage::writeHeader(std::ofstream& out, const std::string& header) {
    out << header << '\n';
}

std::vector<std::string> CSVStorage::splitLine(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        auto start = field.find_first_not_of(" \t\r");
        auto end = field.find_last_not_of(" \t\r");
        if (start == std::string::npos) fields.emplace_back();
        else fields.push_back(field.substr(start, end - start + 1));
    }
    return fields;
}

bool CSVStorage::readHeader(std::ifstream& in, const std::string& expected) {
    std::string line;
    if (!std::getline(in, line)) return false;
    return line.find(expected) == 0;
}

std::string CSVStorage::timePointToString(const std::chrono::system_clock::time_point& tp) {
    return std::to_string(std::chrono::system_clock::to_time_t(tp));
}

std::chrono::system_clock::time_point CSVStorage::stringToTimePoint(const std::string& s) {
    std::time_t t = static_cast<std::time_t>(std::stoll(s));
    return std::chrono::system_clock::from_time_t(t);
}

std::string CSVStorage::yearMonthDayToString(const std::chrono::year_month_day& ymd) {
    auto y = static_cast<int>(ymd.year());
    auto m = static_cast<unsigned>(ymd.month());
    auto d = static_cast<unsigned>(ymd.day());
    std::string result = std::to_string(y) + '-';
    if (m < 10) result += '0';
    result += std::to_string(m) + '-';
    if (d < 10) result += '0';
    result += std::to_string(d);
    return result;
}

std::chrono::year_month_day CSVStorage::stringToYearMonthDay(const std::string& s) {
    int y = 0, m = 1, d = 1;
    char dash1, dash2;
    std::stringstream ss(s);
    ss >> y >> dash1 >> m >> dash2 >> d;
    return std::chrono::year{y} / std::chrono::month{static_cast<unsigned>(m)}
           / std::chrono::day{static_cast<unsigned>(d)};
}

std::string CSVStorage::join(const std::vector<std::string>& parts, char delim) {
    if (parts.empty()) return {};
    std::string result;
    for (size_t i = 0; i < parts.size(); ++i) {
        if (i > 0) result += delim;
        result += parts[i];
    }
    return result;
}

std::vector<std::string> CSVStorage::split(const std::string& s, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(s);
    std::string part;
    while (std::getline(ss, part, delim)) parts.push_back(part);
    return parts;
}

// ─── Players ─────────────────────────────────────────────────────────

static const char* PLAYER_HEADER =
    "playerid,name,username,balance,gamesplayed,totalwagered,"
    "totalwon,totallost,cwins,closs,biggestwin,biggestloss,"
    "fraud,risk,rank,creation,passwordhash,role,age";

bool CSVStorage::savePlayers(const std::vector<player>& players) {
    std::ofstream out;
    if (!openOutput("players.csv", out)) return false;
    writeHeader(out, PLAYER_HEADER);
    for (const auto& p : players) {
        out << p.getplayerid() << ','
            << p.getname() << ','
            << p.getusername() << ','
            << p.getbal() << ','
            << p.getgamesplayed() << ','
            << p.gettotalwagered() << ','
            << p.gettwon() << ','
            << p.gettlost() << ','
            << p.getcwins() << ','
            << p.getcloss() << ','
            << p.getbwin() << ','
            << p.getbloss() << ','
            << (p.isfraud() ? '1' : '0') << ','
            << p.getrisk() << ','
            << p.getrank() << ','
            << timePointToString(p.getcreation()) << ','
            << p.getPasswordHash() << ','
            << p.getRole() << ','
            << p.getAge() << '\n';
    }
    return out.good();
}

std::vector<player> CSVStorage::loadPlayers() {
    std::vector<player> result;
    std::ifstream in;
    if (!openInput("players.csv", in)) return result;
    if (!readHeader(in, PLAYER_HEADER)) return result;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto fields = splitLine(line);
        if (fields.size() < 19) continue;
        try {
            player p(fields[0], fields[1], fields[2], std::stod(fields[3]),
                     fields[16], fields[17], std::stoi(fields[18]));
            p.setgamesplayed(std::stoll(fields[4]));
            p.settotalwagered(std::stod(fields[5]));
            p.settwon(std::stod(fields[6]));
            p.settlost(std::stod(fields[7]));
            p.setcwins(std::stoi(fields[8]));
            p.setcloss(std::stoi(fields[9]));
            p.setbwin(std::stod(fields[10]));
            p.setbloss(std::stod(fields[11]));
            p.setfruad(fields[12] == "1");
            p.setrisk(std::stod(fields[13]));
            p.setrank(std::stoi(fields[14]));
            result.push_back(p);
        } catch (...) { continue; }
    }
    return result;
}

// ─── Bets ────────────────────────────────────────────────────────────

static const char* BET_HEADER =
    "betid,playerid,betamount,date,bettype,risk,expectedvalue,recommended,prediction";

bool CSVStorage::saveBets(const std::vector<Bet>& bets) {
    std::ofstream out;
    if (!openOutput("bets.csv", out)) return false;
    writeHeader(out, BET_HEADER);
    for (const auto& b : bets) {
        auto pred = b.getprediction();
        std::string predStr;
        for (size_t i = 0; i < pred.size(); ++i) {
            if (i > 0) predStr += ';';
            predStr += pred[i];
        }
        out << b.getbetid() << ','
            << b.getpid() << ','
            << b.getbetamt() << ','
            << yearMonthDayToString(b.gettime()) << ','
            << BetTypeToString(b.getbtype()) << ','
            << b.getrisk() << ','
            << b.getexpvalue() << ','
            << b.getrecamt() << ','
            << predStr << '\n';
    }
    return out.good();
}

std::vector<Bet> CSVStorage::loadBets() {
    std::vector<Bet> result;
    std::ifstream in;
    if (!openInput("bets.csv", in)) return result;
    if (!readHeader(in, BET_HEADER)) return result;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto fields = splitLine(line);
        if (fields.size() < 9) continue;
        try {
            auto ymd = stringToYearMonthDay(fields[3]);
            BetType bt = BetTypeFromString(fields[4]);
            std::vector<std::string> prediction;
            if (!fields[8].empty()) {
                std::stringstream pss(fields[8]);
                std::string p;
                while (std::getline(pss, p, ';')) prediction.push_back(p);
            }
            Bet b(fields[0], fields[1], std::stod(fields[2]), ymd,
                  bt, prediction, std::stoi(fields[5]),
                  std::stod(fields[6]), std::stod(fields[7]));
            result.push_back(b);
        } catch (...) { continue; }
    }
    return result;
}

// ─── Game Records ────────────────────────────────────────────────────

static const char* GAME_HEADER =
    "gameid,playerid,betid,spinid,timestamp,betamt,bettype,prediction,"
    "spinsymbols,win,payout,balbefore,balafter,fraud";

bool CSVStorage::saveGames(const std::vector<GameRecord>& games) {
    std::ofstream out;
    if (!openOutput("games.csv", out)) return false;
    writeHeader(out, GAME_HEADER);
    for (const auto& g : games) {
        auto pred = g.getprediction();
        std::string predStr;
        for (size_t i = 0; i < pred.size(); ++i) {
            if (i > 0) predStr += ';';
            predStr += pred[i];
        }
        out << g.getgameid() << ','
            << g.getplayerid() << ','
            << g.getbetid() << ','
            << g.getspinid() << ','
            << g.gettime() << ','
            << g.getbetamt() << ','
            << BetTypeToString(g.getbettype()) << ','
            << predStr << ','
            << join(g.getspinsymbols(), ';') << ','
            << (g.iswin() ? '1' : '0') << ','
            << g.getpayout() << ','
            << g.getbalbefore() << ','
            << g.getbalafter() << ','
            << g.getfraud() << '\n';
    }
    return out.good();
}

std::vector<GameRecord> CSVStorage::loadGames() {
    std::vector<GameRecord> result;
    std::ifstream in;
    if (!openInput("games.csv", in)) return result;
    if (!readHeader(in, GAME_HEADER)) return result;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto fields = splitLine(line);
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
            result.push_back(rec);
        } catch (...) { continue; }
    }
    return result;
}

// ─── Strategy Reports ────────────────────────────────────────────────

static const char* REPORT_HEADER =
    "playerid,time,currbal,recomstrategy,recomamt,expectreturn,risk,"
    "dp,greedy,kelly,confidence";

bool CSVStorage::saveReports(const std::vector<strategyreport>& reports) {
    std::ofstream out;
    if (!openOutput("reports.csv", out)) return false;
    writeHeader(out, REPORT_HEADER);
    for (const auto& r : reports) {
        out << r.getplayerid() << ','
            << timePointToString(r.gettime()) << ','
            << r.getcurrbal() << ','
            << r.getrecomstrategy() << ','
            << r.getrecomamt() << ','
            << r.getexpectreturn() << ','
            << r.getrisk() << ','
            << r.getdp() << ','
            << r.getgreedy() << ','
            << r.getkelly() << ','
            << r.getconfidence() << '\n';
    }
    return out.good();
}

std::vector<strategyreport> CSVStorage::loadReports() {
    std::vector<strategyreport> result;
    std::ifstream in;
    if (!openInput("reports.csv", in)) return result;
    if (!readHeader(in, REPORT_HEADER)) return result;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto fields = splitLine(line);
        if (fields.size() < 11) continue;
        try {
            strategyreport r(fields[0], std::stod(fields[2]), fields[3],
                             std::stod(fields[4]), std::stod(fields[5]),
                             std::stod(fields[6]), fields[7], fields[8],
                             fields[9], std::stod(fields[10]));
            result.push_back(r);
        } catch (...) { continue; }
    }
    return result;
}
