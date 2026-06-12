#include <cassert>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "GameLogger.h"
#include "GameRecord.h"

// ── Helpers ──────────────────────────────────────────────────────────

static GameRecord makeRecord(int gid, const std::string& pid,
                             const std::string& ts, double amt = 10.0) {
    std::vector<std::string> syms = {"CHERRY", "LEMON", "BAR"};
    return GameRecord(gid, pid, "b" + std::to_string(gid),
                      "s" + std::to_string(gid), ts, amt, "standard",
                      syms, true, 15.0, 100.0, 115.0, 0.0);
}

static std::string tmpPath() {
    const char* dir = std::getenv("TEMP");
    if (!dir) dir = ".";
    return std::string(dir) + "/gamelogger_test.csv";
}

// ── Tests ────────────────────────────────────────────────────────────

static void test_empty_logger() {
    GameLogger log;
    assert(log.size() == 0);
    assert(log.getAll().empty());
    assert(log.getPlayerHistory("nonexistent").empty());
    assert(log.getRecentGames(5).empty());
    assert(log.findGameById(42) == -1);
    assert(log.getByTimeRange("a", "z").empty());
    std::cout << "  PASS empty_logger\n";
}

static void test_log_single_game() {
    GameLogger log;
    log.logGame(makeRecord(1, "p1", "2026-01-01"));
    assert(log.size() == 1);
    assert(log.findGameById(1) == 0);
    assert(log.getPlayerHistory("p1").size() == 1);
    assert(log.getRecentGames(1).size() == 1);
    std::cout << "  PASS log_single_game\n";
}

static void test_log_multiple_games() {
    GameLogger log;
    log.logGame(makeRecord(1, "p1", "2026-01-01"));
    log.logGame(makeRecord(2, "p2", "2026-01-02"));
    log.logGame(makeRecord(3, "p1", "2026-01-03"));
    assert(log.size() == 3);
    assert(log.getPlayerHistory("p1").size() == 2);
    assert(log.getPlayerHistory("p2").size() == 1);
    std::cout << "  PASS log_multiple_games\n";
}

static void test_player_history() {
    GameLogger log;
    log.logGame(makeRecord(1, "alice", "2026-01-01"));
    log.logGame(makeRecord(2, "bob",   "2026-01-02"));
    log.logGame(makeRecord(3, "alice", "2026-01-03"));
    log.logGame(makeRecord(4, "alice", "2026-01-04"));

    auto hist = log.getPlayerHistory("alice");
    assert(hist.size() == 3);
    assert(hist[0].getgameid() == 1);
    assert(hist[1].getgameid() == 3);
    assert(hist[2].getgameid() == 4);

    assert(log.getPlayerHistory("bob").size() == 1);
    assert(log.getPlayerHistory("charlie").empty());
    std::cout << "  PASS player_history\n";
}

static void test_recent_games() {
    GameLogger log;
    log.logGame(makeRecord(1, "p1", "t1"));
    log.logGame(makeRecord(2, "p2", "t2"));
    log.logGame(makeRecord(3, "p3", "t3"));
    log.logGame(makeRecord(4, "p4", "t4"));
    log.logGame(makeRecord(5, "p5", "t5"));

    auto recent = log.getRecentGames(3);
    assert(recent.size() == 3);
    assert(recent[0].getgameid() == 3);
    assert(recent[1].getgameid() == 4);
    assert(recent[2].getgameid() == 5);

    auto all = log.getRecentGames(10);
    assert(all.size() == 5);
    std::cout << "  PASS recent_games\n";
}

static void test_find_by_game_id() {
    GameLogger log;
    log.logGame(makeRecord(10, "p1", "t1"));
    log.logGame(makeRecord(20, "p2", "t2"));
    log.logGame(makeRecord(30, "p3", "t3"));

    assert(log.findGameById(20) == 1);
    assert(log.findGameById(10) == 0);
    assert(log.findGameById(30) == 2);
    assert(log.findGameById(99) == -1);
    std::cout << "  PASS find_by_game_id\n";
}

static void test_time_range() {
    GameLogger log;
    log.logGame(makeRecord(1, "p1", "2026-01-01"));
    log.logGame(makeRecord(2, "p2", "2026-01-05"));
    log.logGame(makeRecord(3, "p3", "2026-01-10"));
    log.logGame(makeRecord(4, "p4", "2026-01-15"));
    log.logGame(makeRecord(5, "p5", "2026-01-20"));

    auto range = log.getByTimeRange("2026-01-05", "2026-01-15");
    assert(range.size() == 3);
    assert(range[0].getgameid() == 2);
    assert(range[1].getgameid() == 3);
    assert(range[2].getgameid() == 4);

    assert(log.getByTimeRange("2026-02-01", "2026-02-28").empty());
    assert(log.getByTimeRange("2025-01-01", "2025-12-31").empty());
    std::cout << "  PASS time_range\n";
}

static void test_save_csv() {
    GameLogger log;
    log.logGame(makeRecord(1, "p1", "2026-01-01", 50.0));
    log.logGame(makeRecord(2, "p2", "2026-01-02", 25.0));

    std::string path = tmpPath();
    assert(log.saveToCSV(path));

    std::ifstream in(path);
    assert(in.is_open());
    std::string header;
    std::getline(in, header);
    assert(header.find("gameid") != std::string::npos);

    int lines = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (!line.empty()) ++lines;
    }
    assert(lines == 2);
    in.close();
    std::remove(path.c_str());
    std::cout << "  PASS save_csv\n";
}

static void test_load_csv() {
    std::string path = tmpPath();
    {
        std::ofstream out(path);
        out << "gameid,playerid,betid,spinid,timestamp,betamt,bettype,"
               "spinsymbols,win,payout,balbefore,balafter,fraud\n";
        out << "1,p1,b1,s1,2026-01-01,50,standard,CHERRY;LEMON;BAR,"
               "1,150,500,650,0\n";
        out << "2,p2,b2,s2,2026-01-02,25,standard,BAR;BAR;SEVEN,"
               "0,0,1000,975,0.5\n";
    }

    GameLogger log;
    size_t n = log.loadFromCSV(path);
    assert(n == 2);
    assert(log.size() == 2);
    assert(log.findGameById(1) != -1);
    assert(log.findGameById(2) != -1);
    assert(log.getPlayerHistory("p1").size() == 1);
    assert(log.getRecentGames(1).size() == 1);

    // Append more
    size_t n2 = log.loadFromCSV(path);
    assert(n2 == 2);
    assert(log.size() == 4);

    std::remove(path.c_str());
    std::cout << "  PASS load_csv\n";
}

static void test_malformed_csv_line() {
    std::string path = tmpPath();
    {
        std::ofstream out(path);
        out << "gameid,playerid,betid,spinid,timestamp,betamt,bettype,"
               "spinsymbols,win,payout,balbefore,balafter,fraud\n";
        out << "1,p1,b1,s1,2026-01-01,50,standard,CHERRY,1,150,500,650,0\n";
        out << "not,a,valid,csv,line\n";
        out << "2,p2,b2,s2,2026-01-02,25,standard,BAR;BAR;SEVEN,"
               "0,0,1000,975,0.5\n";
    }

    GameLogger log;
    size_t n = log.loadFromCSV(path);
    assert(n == 2);
    assert(log.size() == 2);
    assert(log.findGameById(1) != -1);
    assert(log.findGameById(2) != -1);

    std::remove(path.c_str());
    std::cout << "  PASS malformed_csv_line\n";
}

static void test_load_nonexistent_csv() {
    GameLogger log;
    size_t n = log.loadFromCSV("/nonexistent/path.csv");
    assert(n == 0);
    assert(log.size() == 0);
    std::cout << "  PASS load_nonexistent_csv\n";
}

static void test_csv_roundtrip() {
    GameLogger log;
    for (int i = 1; i <= 5; ++i) {
        log.logGame(makeRecord(i, "p" + std::to_string(i % 3 + 1),
                               "2026-06-" + std::to_string(10 + i)));
    }
    assert(log.size() == 5);

    std::string path = tmpPath();
    assert(log.saveToCSV(path));

    GameLogger log2;
    assert(log2.loadFromCSV(path) == 5);
    assert(log2.size() == 5);

    // Verify data integrity
    for (int i = 1; i <= 5; ++i) {
        int idx = log2.findGameById(i);
        assert(idx != -1);
        assert(log2.getAll()[static_cast<size_t>(idx)].getplayerid() ==
               "p" + std::to_string(i % 3 + 1));
    }

    assert(log2.getPlayerHistory("p1").size() == 2);
    assert(log2.getPlayerHistory("p2").size() == 2);
    assert(log2.getPlayerHistory("p3").size() == 1);
    assert(log2.getPlayerHistory("p4").empty());

    std::remove(path.c_str());
    std::cout << "  PASS csv_roundtrip\n";
}

// ── Main ─────────────────────────────────────────────────────────────

int main() {
    std::cout << "GameLogger tests:\n";

    test_empty_logger();
    test_log_single_game();
    test_log_multiple_games();
    test_player_history();
    test_recent_games();
    test_find_by_game_id();
    test_time_range();
    test_save_csv();
    test_load_csv();
    test_malformed_csv_line();
    test_load_nonexistent_csv();
    test_csv_roundtrip();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
