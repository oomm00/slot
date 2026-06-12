#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Bet.h"
#include "BetType.h"
#include "BettingEngine.h"
#include "CSVStorage.h"
#include "GameLogger.h"
#include "GameRecord.h"
#include "Player.h"
#include "PlayerManager.h"
#include "SearchEngine.h"
#include "SlotMachine.h"

#include "BinarySearch.h"
#include "KMP.h"
#include "RabinKarp.h"

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::string dir = std::string(tmp) + "/searchengine_test/" + std::to_string(now);
    std::filesystem::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    std::filesystem::remove_all(dir);
}

static bool approx(double a, double b, double eps = 0.001) {
    return std::abs(a - b) < eps;
}

// ═════════════════════════════════════════════════════════════════════
//  BinarySearch Tests
// ═════════════════════════════════════════════════════════════════════

static void test_bs_exact_found() {
    int arr[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    BinarySearch<int> bs;
    int pos = bs.search(arr, arr + 10, 23);
    assert(pos == 5);
    pos = bs.search(arr, arr + 10, 2);
    assert(pos == 0);
    pos = bs.search(arr, arr + 10, 91);
    assert(pos == 9);
    std::cout << "  PASS bs_exact_found\n";
}

static void test_bs_not_found() {
    int arr[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    BinarySearch<int> bs;
    assert(bs.search(arr, arr + 10, 1) == -1);
    assert(bs.search(arr, arr + 10, 100) == -1);
    assert(bs.search(arr, arr + 10, 10) == -1);
    std::cout << "  PASS bs_not_found\n";
}

static void test_bs_empty() {
    BinarySearch<int> bs;
    assert(bs.search(nullptr, nullptr, 42) == -1);
    std::cout << "  PASS bs_empty\n";
}

static void test_bs_strings() {
    std::vector<std::string> ids = {"bet-001", "bet-002", "bet-003", "bet-010"};
    BinarySearch<std::string> bs;
    assert(bs.search(ids.data(), ids.data() + ids.size(), "bet-002") == 1);
    assert(bs.search(ids.data(), ids.data() + ids.size(), "bet-999") == -1);
    std::cout << "  PASS bs_strings\n";
}

static void test_bs_lower_bound() {
    int arr[] = {1, 3, 5, 7, 9};
    BinarySearch<int> bs;
    assert(bs.lowerBound(arr, arr + 5, 4) == 2);  // first >= 4 is 5 at index 2
    assert(bs.lowerBound(arr, arr + 5, 10) == 5); // past end
    std::cout << "  PASS bs_lower_bound\n";
}

// ═════════════════════════════════════════════════════════════════════
//  KMP Tests
// ═════════════════════════════════════════════════════════════════════

static void test_kmp_exact_match() {
    KMP<> kmp;
    assert(kmp.search("hello world", "world") == 6);
    assert(kmp.search("abcabc", "abc") == 0);
    std::cout << "  PASS kmp_exact_match\n";
}

static void test_kmp_no_match() {
    KMP<> kmp;
    assert(kmp.search("hello world", "xyz") == -1);
    std::cout << "  PASS kmp_no_match\n";
}

static void test_kmp_empty() {
    KMP<> kmp;
    assert(kmp.search("hello", "") == 0);
    std::cout << "  PASS kmp_empty\n";
}

static void test_kmp_pattern_longer_than_text() {
    KMP<> kmp;
    assert(kmp.search("abc", "abcdef") == -1);
    std::cout << "  PASS kmp_pattern_longer\n";
}

static void test_kmp_all_matches() {
    KMP<> kmp;
    auto matches = kmp.searchAll("aaaaa", "aa");
    assert(matches.size() == 4);
    assert(matches[0] == 0);
    assert(matches[1] == 1);
    assert(matches[2] == 2);
    assert(matches[3] == 3);
    std::cout << "  PASS kmp_all_matches\n";
}

static void test_kmp_symbol_sequence() {
    // Vector-based KMP for symbol sequences.
    KMP<std::vector<std::string>> kmp;
    std::vector<std::string> text = {"CHERRY", "LEMON", "LEMON", "SEVEN", "BELL"};
    std::vector<std::string> pattern = {"LEMON", "SEVEN"};
    assert(kmp.search(text, pattern) == 2);
    std::cout << "  PASS kmp_symbol_sequence\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Rabin-Karp Tests
// ═════════════════════════════════════════════════════════════════════

static void test_rk_exact_match() {
    RabinKarp<std::string> rk;
    assert(rk.search("hello world", "world") == 6);
    assert(rk.search("abcabc", "abc") == 0);
    std::cout << "  PASS rk_exact_match\n";
}

static void test_rk_no_match() {
    RabinKarp<std::string> rk;
    assert(rk.search("hello world", "xyz") == -1);
    std::cout << "  PASS rk_no_match\n";
}

static void test_rk_empty() {
    RabinKarp<std::string> rk;
    assert(rk.search("hello", "") == 0);
    std::cout << "  PASS rk_empty\n";
}

static void test_rk_all_matches() {
    RabinKarp<std::string> rk;
    auto matches = rk.searchAll("aaaaa", "aa");
    assert(matches.size() == 4);
    std::cout << "  PASS rk_all_matches\n";
}

static void test_rk_double_vector() {
    RabinKarp<std::vector<double>> rk;
    std::vector<double> text = {10.0, 20.0, 30.0, 40.0, 50.0};
    std::vector<double> pattern = {30.0, 40.0};
    assert(rk.search(text, pattern) == 2);
    std::cout << "  PASS rk_double_vector\n";
}

static void test_rk_double_no_match() {
    RabinKarp<std::vector<double>> rk;
    std::vector<double> text = {10.0, 20.0, 30.0};
    std::vector<double> pattern = {99.0, 100.0};
    assert(rk.search(text, pattern) == -1);
    std::cout << "  PASS rk_double_no_match\n";
}

// ═════════════════════════════════════════════════════════════════════
//  SearchEngine Integration Tests
// ═════════════════════════════════════════════════════════════════════

static void test_se_search_player_by_id() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    pm.addPlayer(player("p2", "Bob", "Bob", 500.0, "", "player", 18));
    pm.addPlayer(player("p10", "Charlie", "Charlie", 250.0, "", "player", 18));

    SearchEngine se(pm, gl, be);

    const player* p = se.searchPlayerById("p1");
    assert(p != nullptr);
    assert(p->getplayerid() == "p1");
    assert(approx(p->getbal(), 1000.0));

    p = se.searchPlayerById("p10");
    assert(p != nullptr);
    assert(p->getplayerid() == "p10");

    p = se.searchPlayerById("nonexistent");
    assert(p == nullptr);

    cleanup(dir);
    std::cout << "  PASS se_search_player_by_id\n";
}

static void test_se_search_player_by_name() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    pm.addPlayer(player("p2", "Alberto", 500.0));
    pm.addPlayer(player("p3", "Bob", "Bob", 250.0, "", "player", 18));

    SearchEngine se(pm, gl, be);

    auto alice = se.searchPlayerByName("Alice");
    assert(alice.size() == 1);
    assert(alice[0].getname() == "Alice");

    auto al = se.searchPlayerByName("Al");
    assert(al.size() == 2);  // Alice, Alberto

    auto z = se.searchPlayerByName("Z");
    assert(z.empty());

    cleanup(dir);
    std::cout << "  PASS se_search_player_by_name\n";
}

static void test_se_search_game_by_id() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));
    for (int i = 0; i < 5; ++i) {
        assert(be.processSpin("p1", 100.0));
    }

    SearchEngine se(pm, gl, be);

    // Game IDs start at 0.
    auto g = se.searchGameById(0);
    assert(g.getplayerid() == "p1");

    g = se.searchGameById(4);
    assert(g.getplayerid() == "p1");

    bool threw = false;
    try {
        se.searchGameById(999);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS se_search_game_by_id\n";
}

static void test_se_search_bet_by_id() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));

    be.placeBet("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 200.0, BetType::ANY_PAIR, {});

    SearchEngine se(pm, gl, be);

    Bet b = se.searchBetById("BET-0");
    assert(b.getbetid() == "BET-0");
    assert(approx(b.getbetamt(), 100.0));

    b = se.searchBetById("BET-1");
    assert(b.getbetid() == "BET-1");
    assert(approx(b.getbetamt(), 200.0));

    bool threw = false;
    try {
        se.searchBetById("BET-999");
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS se_search_bet_by_id\n";
}

static void test_se_search_games_by_date_range() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));
    for (int i = 0; i < 3; ++i) {
        assert(be.processSpin("p1", 100.0));
    }

    SearchEngine se(pm, gl, be);

    auto games = se.searchGamesByDateRange("2000-01-01", "2099-12-31");
    assert(games.size() == 3);

    auto none = se.searchGamesByDateRange("1990-01-01", "1990-12-31");
    assert(none.empty());

    cleanup(dir);
    std::cout << "  PASS se_search_games_by_date_range\n";
}

static void test_se_empty_datasets() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    SearchEngine se(pm, gl, be);

    assert(se.searchPlayerById("any") == nullptr);

    auto names = se.searchPlayerByName("test");
    assert(names.empty());

    bool threw = false;
    try {
        se.searchGameById(0);
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    threw = false;
    try {
        se.searchBetById("BET-0");
    } catch (const std::out_of_range&) {
        threw = true;
    }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS se_empty_datasets\n";
}

static void test_se_missing_records() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    pm.addPlayer(player("p2", "Bob", "Bob", 500.0, "", "player", 18));
    be.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p2", 50.0, BetType::ANY_PAIR, {});

    SearchEngine se(pm, gl, be);

    // Non-existent player
    assert(se.searchPlayerById("p999") == nullptr);

    // Non-existent game
    bool threw = false;
    try { se.searchGameById(999); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    // Non-existent bet
    threw = false;
    try { se.searchBetById("BET-999"); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS se_missing_records\n";
}

static void test_se_symbol_pattern() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 100000.0, "", "player", 18));

    // Run many spins to create symbol variety.
    for (int i = 0; i < 50; ++i) {
        be.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    }

    SearchEngine se(pm, gl, be);

    // Search for a common symbol pattern (single symbol = matches any game with that symbol)
    // We just verify the API works and returns results or empty — KMP correctness
    // is validated in standalone tests above.
    auto matches = se.searchSymbolPattern({"CHERRY"});
    // May be zero if no game had CHERRY, but at 50 spins it's very likely.
    // Just verify the API doesn't crash.

    // Empty pattern should return empty.
    auto empty = se.searchSymbolPattern({});
    assert(empty.empty());

    cleanup(dir);
    std::cout << "  PASS se_symbol_pattern\n";
}

static void test_se_bet_pattern() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    pm.addPlayer(player("p1", "Alice", "Alice", 100000.0, "", "player", 18));

    be.placeBet("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 200.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 200.0, BetType::ANY_PAIR, {});

    SearchEngine se(pm, gl, be);

    auto matches = se.searchBetPattern({100.0, 200.0});
    assert(!matches.empty());

    // Each match should be a sequence of 2 bets with amounts 100.0, 200.0
    for (const auto& seq : matches) {
        assert(seq.size() == 2);
        assert(approx(seq[0].getbetamt(), 100.0));
        assert(approx(seq[1].getbetamt(), 200.0));
    }

    // Pattern not present.
    auto none = se.searchBetPattern({999.0, 888.0});
    assert(none.empty());

    cleanup(dir);
    std::cout << "  PASS se_bet_pattern\n";
}

static void test_se_rebuild_indices() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    GameLogger gl;
    SlotMachine sm;
    BettingEngine be(pm, sm, gl);

    SearchEngine se(pm, gl, be);

    // Add data after construction, then rebuild.
    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    be.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    be.placeBet("p1", 50.0, BetType::ANY_PAIR, {});

    se.rebuildIndices();

    const player* p = se.searchPlayerById("p1");
    assert(p != nullptr);

    auto g = se.searchGameById(0);
    assert(g.getplayerid() == "p1");

    auto b = se.searchBetById("BET-0");
    assert(b.getbetid() == "BET-0");

    cleanup(dir);
    std::cout << "  PASS se_rebuild_indices\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "BinarySearch tests:\n";
    test_bs_exact_found();
    test_bs_not_found();
    test_bs_empty();
    test_bs_strings();
    test_bs_lower_bound();

    std::cout << "\nKMP tests:\n";
    test_kmp_exact_match();
    test_kmp_no_match();
    test_kmp_empty();
    test_kmp_pattern_longer_than_text();
    test_kmp_all_matches();
    test_kmp_symbol_sequence();

    std::cout << "\nRabinKarp tests:\n";
    test_rk_exact_match();
    test_rk_no_match();
    test_rk_empty();
    test_rk_all_matches();
    test_rk_double_vector();
    test_rk_double_no_match();

    std::cout << "\nSearchEngine integration tests:\n";
    test_se_search_player_by_id();
    test_se_search_player_by_name();
    test_se_search_game_by_id();
    test_se_search_bet_by_id();
    test_se_search_games_by_date_range();
    test_se_empty_datasets();
    test_se_missing_records();
    test_se_symbol_pattern();
    test_se_bet_pattern();
    test_se_rebuild_indices();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
