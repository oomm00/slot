#include <cassert>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "BetType.h"
#include "CSVStorage.h"

// ─── Test helpers ────────────────────────────────────────────────────

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    std::string dir = std::string(tmp) + "/csvstorage_test/"
        + std::to_string(std::chrono::system_clock::to_time_t(
            std::chrono::system_clock::now()));
    std::filesystem::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    std::filesystem::remove_all(dir);
}

// ─── 1. Player save/load ─────────────────────────────────────────────

static void test_save_load_players() {
    auto dir = tempDir();
    CSVStorage store(dir);

    std::vector<player> players;
    players.emplace_back("p1", "Alice",   1000.0);
    players.emplace_back("p2", "Bob",     500.0);
    players.emplace_back("p3", "Charlie", 250.0);

    // Modify some fields to verify round-trip
    players[0].setfruad(true);
    players[0].setrisk(0.75);
    players[0].setrank(3);
    players[1].setrisk(0.25);
    players[1].setrank(1);
    players[2].setfruad(false);
    players[2].setrisk(0.5);
    players[2].setrank(2);

    assert(store.savePlayers(players));

    auto loaded = store.loadPlayers();
    assert(loaded.size() == 3);

    // Verify fields that round-trip
    assert(loaded[0].getplayerid() == "p1");
    assert(loaded[0].getname() == "Alice");
    assert(loaded[1].getplayerid() == "p2");
    assert(loaded[1].getname() == "Bob");
    assert(loaded[2].getplayerid() == "p3");
    assert(loaded[2].getname() == "Charlie");

    assert(loaded[0].isfraud() == true);
    assert(loaded[2].isfraud() == false);
    assert(loaded[1].getrisk() == 0.25);
    assert(loaded[1].getrank() == 1);

    cleanup(dir);
    std::cout << "  PASS save_load_players\n";
}

// ─── 2. Bet save/load ────────────────────────────────────────────────

static void test_save_load_bets() {
    auto dir = tempDir();
    CSVStorage store(dir);

    using namespace std::chrono;
    std::vector<Bet> bets;
    bets.emplace_back("b1", "p1", 100.0, year_month_day{year{2026}/6/1}, BetType::EXACT_PREDICTION, {}, 50, 120.0, 80.0);
    bets.emplace_back("b2", "p2",  50.0, year_month_day{year{2026}/6/2}, BetType::TRIPLE_SYMBOL, {}, 30,  60.0, 40.0);
    bets.emplace_back("b3", "p3", 200.0, year_month_day{year{2026}/6/3}, BetType::PAIR_PREDICTION, {}, 70, 250.0, 150.0);

    assert(store.saveBets(bets));

    auto loaded = store.loadBets();
    assert(loaded.size() == 3);

    assert(loaded[0].getbetid() == "b1");
    assert(loaded[0].getpid() == "p1");
    assert(loaded[0].getbetamt() == 100.0);
    assert(loaded[0].gettime() == year_month_day{year{2026}/6/1});
    assert(loaded[0].getbtype() == BetType::EXACT_PREDICTION);
    assert(loaded[0].getrisk() == 50);
    assert(loaded[0].getexpvalue() == 120.0);
    assert(loaded[0].getrecamt() == 80.0);

    assert(loaded[1].getbtype() == BetType::TRIPLE_SYMBOL);
    assert(loaded[2].getbtype() == BetType::PAIR_PREDICTION);

    cleanup(dir);
    std::cout << "  PASS save_load_bets\n";
}

// ─── 3. GameRecord save/load ─────────────────────────────────────────

static void test_save_load_games() {
    auto dir = tempDir();
    CSVStorage store(dir);

    std::vector<GameRecord> games;
    games.emplace_back(1, "p1", "b1", "s1", "2026-06-01 10:00:00", 50.0,
                       BetType::ANY_PAIR, {}, std::vector<std::string>{"CHERRY","LEMON","BAR"},
                       true, 150.0, 500.0, 650.0, 0.0);
    games.emplace_back(2, "p2", "b2", "s2", "2026-06-01 10:05:00", 25.0,
                       BetType::ANY_PAIR, {}, std::vector<std::string>{"SEVEN","SEVEN","BAR"},
                       false, 0.0, 1000.0, 975.0, 0.0);
    games.emplace_back(3, "p1", "b3", "s3", "2026-06-01 10:10:00", 100.0,
                       BetType::ANY_PAIR, {}, std::vector<std::string>{"CHERRY","CHERRY","CHERRY"},
                       true, 300.0, 650.0, 950.0, 0.0);

    assert(store.saveGames(games));

    auto loaded = store.loadGames();
    assert(loaded.size() == 3);

    assert(loaded[0].getgameid() == 1);
    assert(loaded[0].getplayerid() == "p1");
    assert(loaded[0].getbetid() == "b1");
    assert(loaded[0].getspinid() == "s1");
    assert(loaded[0].gettime() == "2026-06-01 10:00:00");
    assert(loaded[0].getbetamt() == 50.0);
    assert(loaded[0].getbettype() == BetType::ANY_PAIR);
    assert(loaded[0].getspinsymbols().size() == 3);
    assert(loaded[0].getspinsymbols()[0] == "CHERRY");
    assert(loaded[0].iswin() == true);
    assert(loaded[0].getpayout() == 150.0);
    assert(loaded[0].getbalbefore() == 500.0);
    assert(loaded[0].getbalafter() == 650.0);

    assert(loaded[1].iswin() == false);
    assert(loaded[2].iswin() == true);
    assert(loaded[2].getgameid() == 3);

    cleanup(dir);
    std::cout << "  PASS save_load_games\n";
}

// ─── 4. StrategyReport save/load ─────────────────────────────────────

static void test_save_load_reports() {
    auto dir = tempDir();
    CSVStorage store(dir);

    std::vector<strategyreport> reports;
    reports.emplace_back("p1", 1000.0, "kelly", 50.0, 120.0, 0.3,
                         "DP:optimal", "Greedy:aggressive", "Kelly:0.25", 0.85);
    reports.emplace_back("p2", 500.0, "dp", 25.0, 60.0, 0.2,
                         "DP:balanced", "Greedy:moderate", "Kelly:0.15", 0.75);

    assert(store.saveReports(reports));

    auto loaded = store.loadReports();
    assert(loaded.size() == 2);

    assert(loaded[0].getplayerid() == "p1");
    assert(loaded[0].getcurrbal() == 1000.0);
    assert(loaded[0].getrecomstrategy() == "kelly");
    assert(loaded[0].getrecomamt() == 50.0);
    assert(loaded[0].getexpectreturn() == 120.0);
    assert(loaded[0].getrisk() == 0.3);
    assert(loaded[0].getdp() == "DP:optimal");
    assert(loaded[0].getgreedy() == "Greedy:aggressive");
    assert(loaded[0].getkelly() == "Kelly:0.25");
    assert(loaded[0].getconfidence() == 0.85);

    assert(loaded[1].getplayerid() == "p2");

    cleanup(dir);
    std::cout << "  PASS save_load_reports\n";
}

// ─── 5. Empty file handling ──────────────────────────────────────────

static void test_empty_file() {
    auto dir = tempDir();
    CSVStorage store(dir);

    auto players = store.loadPlayers();
    assert(players.empty());

    auto bets = store.loadBets();
    assert(bets.empty());

    auto games = store.loadGames();
    assert(games.empty());

    auto reports = store.loadReports();
    assert(reports.empty());

    cleanup(dir);
    std::cout << "  PASS empty_file\n";
}

// ─── 6. Corrupted row handling ───────────────────────────────────────

static void test_corrupted_rows() {
    auto dir = tempDir();
    CSVStorage store(dir);

    // Create a CSV file with mixed valid/invalid rows manually
    {
        std::ofstream out(dir + "/players.csv");
        out << "playerid,username,balance,gamesplayed,totalwagered,"
               "totalwon,totallost,cwins,closs,biggestwin,biggestloss,"
               "fraud,risk,rank,creation\n";
        out << "p1,Alice,1000,0,0,0,0,0,0,0,0,0,0,0,1000000\n";
        out << "this,is,a,garbage,line,with,too,few,cols\n";
        out << "p2,Bob,500,0,0,0,0,0,0,0,0,0,0,0,1000001\n";
        out << ",,,,,,,,,,,,,\n";
        out << "notanumber,Charlie,abc,,,,,,,,,,,\n";
        out << "p3,Dave,250,0,0,0,0,0,0,0,0,1,0.5,2,1000002\n";
    }

    auto loaded = store.loadPlayers();
    assert(loaded.size() == 3);
    assert(loaded[0].getplayerid() == "p1");
    assert(loaded[1].getplayerid() == "p2");
    assert(loaded[2].getplayerid() == "p3");

    cleanup(dir);
    std::cout << "  PASS corrupted_rows\n";
}

// ─── 7. Multiple entities round-trip ─────────────────────────────────

static void test_multi_entity_roundtrip() {
    auto dir = tempDir();
    CSVStorage store(dir);

    // Players
    std::vector<player> players;
    players.emplace_back("p1", "Alice", 1000.0);
    assert(store.savePlayers(players));
    assert(store.loadPlayers().size() == 1);

    // Bets
    using namespace std::chrono;
    std::vector<Bet> bets;
    bets.emplace_back("b1", "p1", 100.0, year_month_day{year{2026}/6/1}, BetType::EXACT_PREDICTION, {}, 50, 120.0, 80.0);
    assert(store.saveBets(bets));
    assert(store.loadBets().size() == 1);

    // Games
    std::vector<GameRecord> games;
    games.emplace_back(1, "p1", "b1", "s1", "2026-06-01", 50.0,
                       BetType::ANY_PAIR, {}, std::vector<std::string>{"CHERRY"} ,
                       true, 150.0, 500.0, 650.0, 0.0);
    assert(store.saveGames(games));
    assert(store.loadGames().size() == 1);

    // Reports
    std::vector<strategyreport> reports;
    reports.emplace_back("p1", 1000.0, "kelly", 50.0, 120.0, 0.3,
                         "DP:optimal", "Greedy:aggressive", "Kelly:0.25", 0.85);
    assert(store.saveReports(reports));
    assert(store.loadReports().size() == 1);

    // Verify no cross-contamination between files
    assert(store.loadPlayers().size() == 1);
    assert(store.loadBets().size() == 1);
    assert(store.loadGames().size() == 1);
    assert(store.loadReports().size() == 1);

    cleanup(dir);
    std::cout << "  PASS multi_entity_roundtrip\n";
}

// ─── 8. Default directory (data/) ────────────────────────────────────

static void test_default_directory() {
    CSVStorage store;
    // Just verify construction works with default "data" dir
    // No assertion on load since files may not exist
    auto players = store.loadPlayers();
    // Should return empty, not crash
    std::cout << "  PASS default_directory\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "CSVStorage tests:\n";

    test_save_load_players();
    test_save_load_bets();
    test_save_load_games();
    test_save_load_reports();
    test_empty_file();
    test_corrupted_rows();
    test_multi_entity_roundtrip();
    test_default_directory();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
