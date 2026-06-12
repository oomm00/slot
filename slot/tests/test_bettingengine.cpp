#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "BettingEngine.h"
#include "CSVStorage.h"
#include "GameLogger.h"
#include "Player.h"
#include "PlayerManager.h"
#include "SlotMachine.h"
#include "BetType.h"

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::string dir = std::string(tmp) + "/bettingengine_test/" + std::to_string(now);
    std::filesystem::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    std::filesystem::remove_all(dir);
}

// ─── Helpers ──────────────────────────────────────────────────────────

static bool approx(double a, double b, double eps = 0.001) {
    return std::abs(a - b) < eps;
}

// ─── 1. validateBet – valid amount ────────────────────────────────────

static void test_validate_bet_valid() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    player* p = pm.getPlayer("p1");
    assert(p != nullptr);

    assert(engine.validateBet(*p, 500.0));
    assert(engine.validateBet(*p, 1000.0));
    assert(engine.validateBet(*p, 0.01));

    cleanup(dir);
    std::cout << "  PASS validate_bet_valid\n";
}

// ─── 2. validateBet – zero amount ─────────────────────────────────────

static void test_validate_bet_zero() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    player* p = pm.getPlayer("p1");
    assert(!engine.validateBet(*p, 0.0));

    cleanup(dir);
    std::cout << "  PASS validate_bet_zero\n";
}

// ─── 3. validateBet – negative amount ─────────────────────────────────

static void test_validate_bet_negative() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    player* p = pm.getPlayer("p1");
    assert(!engine.validateBet(*p, -100.0));
    assert(!engine.validateBet(*p, -0.01));

    cleanup(dir);
    std::cout << "  PASS validate_bet_negative\n";
}

// ─── 4. validateBet – exceeds balance ─────────────────────────────────

static void test_validate_bet_exceeds_balance() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 500.0, "", "player", 18));
    player* p = pm.getPlayer("p1");
    assert(!engine.validateBet(*p, 500.01));
    assert(!engine.validateBet(*p, 1000.0));

    cleanup(dir);
    std::cout << "  PASS validate_bet_exceeds_balance\n";
}

// ─── 5. placeBet – success ────────────────────────────────────────────

static void test_place_bet() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));

    Bet bet = engine.placeBet("p1", 200.0, BetType::ANY_PAIR, {});

    assert(bet.getbetid() == "BET-0");
    assert(bet.getpid() == "p1");
    assert(approx(bet.getbetamt(), 200.0));
    assert(bet.getbtype() == BetType::EXACT_PREDICTION);
    assert(bet.getrisk() == 50);
    assert(approx(bet.getexpvalue(), 190.0));

    // Balance deducted
    player* p = pm.getPlayer("p1");
    assert(approx(p->getbal(), 800.0));

    cleanup(dir);
    std::cout << "  PASS place_bet\n";
}

// ─── 6. placeBet – invalid player ─────────────────────────────────────

static void test_place_bet_invalid_player() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    bool threw = false;
    try {
        engine.placeBet("nonexistent", 100.0, BetType::ANY_PAIR, {});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS place_bet_invalid_player\n";
}

// ─── 7. placeBet – insufficient funds ─────────────────────────────────

static void test_place_bet_insufficient_funds() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 100.0, "", "player", 18));

    bool threw = false;
    try {
        engine.placeBet("p1", 200.0, BetType::ANY_PAIR, {});
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);

    cleanup(dir);
    std::cout << "  PASS place_bet_insufficient_funds\n";
}

// ─── 8. placeBet – unknown bet type ───────────────────────────────────
// Removed: string-based bet types no longer supported
static void test_place_bet_unknown_type() {
    // This test is disabled because string-based bet types are no longer supported.
}

// ─── 9. calculatePayout ───────────────────────────────────────────────

static void test_calculate_payout() {
    assert(approx(BettingEngine::calculatePayout(100.0, 2.0), 200.0));
    assert(approx(BettingEngine::calculatePayout(50.0, 0.5), 25.0));
    assert(approx(BettingEngine::calculatePayout(200.0, 0.0), 0.0));
    assert(approx(BettingEngine::calculatePayout(0.0, 5.0), 0.0));
    assert(approx(BettingEngine::calculatePayout(77.77, 3.0), 233.31));

    std::cout << "  PASS calculate_payout\n";
}

// ─── 10. processSpin – success ─────────────────────────────────────────

static void test_process_spin_success() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));

    bool ok = engine.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    assert(ok);

    player* p = pm.getPlayer("p1");
    // Balance must have changed (min: deducted 100, could be higher if won)
    assert(!approx(p->getbal(), 1000.0));

    // Stats updated
    auto stats = engine.getStats();
    assert(stats.totalBets == 1);
    assert(approx(stats.totalWagered, 100.0));
    assert(stats.totalPayouts >= 0.0);
    assert(approx(stats.houseProfit, stats.totalWagered - stats.totalPayouts));

    // Game logged
    assert(logger.size() == 1);
    auto recent = logger.getRecentGames(1);
    assert(recent.size() == 1);
    assert(recent[0].getplayerid() == "p1");
    assert(approx(recent[0].getbetamt(), 100.0));

    // Bet recorded
    auto bets = engine.getPlayerBets("p1");
    assert(bets.size() == 1);
    assert(bets[0].getpid() == "p1");
    assert(approx(bets[0].getbetamt(), 100.0));

    cleanup(dir);
    std::cout << "  PASS process_spin_success\n";
}

// ─── 11. processSpin – invalid player ─────────────────────────────────

static void test_process_spin_invalid_player() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    bool ok = engine.processSpin("nonexistent", 100.0, BetType::ANY_PAIR, {});
    assert(!ok);

    cleanup(dir);
    std::cout << "  PASS process_spin_invalid_player\n";
}

// ─── 12. processSpin – insufficient funds ─────────────────────────────

static void test_process_spin_insufficient_funds() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 50.0, "", "player", 18));
    bool ok = engine.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    assert(!ok);

    cleanup(dir);
    std::cout << "  PASS process_spin_insufficient_funds\n";
}

// ─── 13. getStats – initial state ─────────────────────────────────────

static void test_stats_initial() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    auto stats = engine.getStats();
    assert(stats.totalBets == 0);
    assert(approx(stats.totalWagered, 0.0));
    assert(approx(stats.totalPayouts, 0.0));
    assert(approx(stats.houseProfit, 0.0));
    assert(approx(stats.winRate, 0.0));

    cleanup(dir);
    std::cout << "  PASS stats_initial\n";
}

// ─── 14. getStats – after multiple spins ──────────────────────────────

static void test_stats_after_spins() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));

    const int N = 20;
    for (int i = 0; i < N; ++i) {
        assert(engine.processSpin("p1", 100.0, BetType::ANY_PAIR, {}));
    }

    auto stats = engine.getStats();
    assert(stats.totalBets == N);
    assert(approx(stats.totalWagered, N * 100.0));
    assert(stats.totalPayouts >= 0.0);
    assert(approx(stats.houseProfit, stats.totalWagered - stats.totalPayouts));
    assert(stats.winRate >= 0.0 && stats.winRate <= 100.0);

    cleanup(dir);
    std::cout << "  PASS stats_after_spins\n";
}

// ─── 15. getPlayerBets – empty ────────────────────────────────────────

static void test_get_player_bets_empty() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    auto bets = engine.getPlayerBets("nonexistent");
    assert(bets.empty());

    cleanup(dir);
    std::cout << "  PASS get_player_bets_empty\n";
}

// ─── 16. getPlayerBets – multiple bets per player ─────────────────────

static void test_get_player_bets_multiple() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 5000.0, "", "player", 18));
    pm.addPlayer(player("p2", "Bob", "Bob", 5000.0, "", "player", 18));

    // p1 places some bets, p2 places some bets
    engine.placeBet("p1", 100.0, BetType::ANY_PAIR, {});
    engine.placeBet("p1", 200.0, BetType::ANY_PAIR, {});
    engine.placeBet("p2", 150.0, BetType::ANY_PAIR, {});
    engine.placeBet("p1", 50.0, BetType::ANY_PAIR, {});

    auto p1bets = engine.getPlayerBets("p1");
    assert(p1bets.size() == 3);
    assert(p1bets[0].getpid() == "p1");
    assert(p1bets[1].getpid() == "p1");
    assert(p1bets[2].getpid() == "p1");

    auto p2bets = engine.getPlayerBets("p2");
    assert(p2bets.size() == 1);
    assert(p2bets[0].getpid() == "p2");
    assert(p2bets[0].getbtype() == BetType::PAIR_PREDICTION); // conservative

    cleanup(dir);
    std::cout << "  PASS get_player_bets_multiple\n";
}

// ─── 17. Mixed: placeBet + processSpin ────────────────────────────────

static void test_mixed_operations() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));

    // Independent placeBet (deducts balance, records Bet)
    Bet b1 = engine.placeBet("p1", 500.0, BetType::ANY_PAIR, {});
    assert(b1.getbetid() == "BET-0");
    assert(approx(pm.getPlayer("p1")->getbal(), 9500.0));

    // processSpin (separate flow, deducts again)
    assert(engine.processSpin("p1", 300.0, BetType::ANY_PAIR, {}));
    assert(!approx(pm.getPlayer("p1")->getbal(), 9200.0)); // changed

    // Both operations recorded
    assert(logger.size() == 1);             // only processSpin logs a game
    auto bets = engine.getPlayerBets("p1");
    assert(bets.size() == 2);              // placeBet + processSpin

    cleanup(dir);
    std::cout << "  PASS mixed_operations\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "BettingEngine tests:\n";

    test_validate_bet_valid();
    test_validate_bet_zero();
    test_validate_bet_negative();
    test_validate_bet_exceeds_balance();
    test_place_bet();
    test_place_bet_invalid_player();
    test_place_bet_insufficient_funds();
    // test_place_bet_unknown_type(); // removed: string-based bet types no longer supported
    test_calculate_payout();
    test_process_spin_success();
    test_process_spin_invalid_player();
    test_process_spin_insufficient_funds();
    test_stats_initial();
    test_stats_after_spins();
    test_get_player_bets_empty();
    test_get_player_bets_multiple();
    test_mixed_operations();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
