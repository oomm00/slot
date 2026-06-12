#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "ApplicationController.h"
#include "Bet.h"
#include "CircularBuffer.h"
#include "FraudDetector.h"
#include "GameRecord.h"
#include "Player.h"
#include "RabinKarpDetector.h"
#include "SlidingWindow.h"
#include "StrategyReport.h"
#include "ZScoreDetector.h"

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::string dir = std::string(tmp) + "/system_test/" + std::to_string(now);
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
//  FraudDetector Sub-component Tests
// ═════════════════════════════════════════════════════════════════════

static void test_circular_buffer() {
    CircularBuffer<int> buf(5);
    assert(buf.empty());
    assert(buf.size() == 0);

    for (int i = 1; i <= 3; ++i) buf.push(i);
    assert(buf.size() == 3);
    assert(buf.get(0) == 1);
    assert(buf.get(2) == 3);

    for (int i = 4; i <= 7; ++i) buf.push(i);
    assert(buf.size() == 5);  // capped
    assert(buf.get(0) == 3);  // 3,4,5,6,7 — oldest is 3

    auto v = buf.toVector();
    assert(v.size() == 5);
    assert(v[0] == 3);

    buf.clear();
    assert(buf.empty());

    std::cout << "  PASS circular_buffer\n";
}

static void test_sliding_window() {
    SlidingWindow<double> win(3);
    assert(win.empty());

    win.push(10.0);
    win.push(20.0);
    win.push(30.0);
    assert(win.size() == 3);
    assert(approx(win.mean(), 20.0));

    win.push(40.0);  // pushes out 10.0
    assert(win.size() == 3);
    assert(approx(win.mean(), 30.0));

    win.clear();
    assert(win.empty());

    std::cout << "  PASS sliding_window\n";
}

static void test_zscore_detector() {
    ZScoreDetector zd(2.0);
    std::vector<double> samples = {10, 11, 10, 12, 11, 10, 11, 10};
    zd.train(samples);

    // A value close to mean should not be anomalous
    assert(!zd.isAnomalous(11.0));

    // A very extreme value should be anomalous
    assert(zd.isAnomalous(100.0));

    // Anomaly score in [0, 1]
    double score = zd.anomalyScore(11.0);
    assert(score >= 0.0 && score <= 1.0);

    score = zd.anomalyScore(100.0);
    assert(score > 0.0);

    std::cout << "  PASS zscore_detector\n";
}

static void test_rabinkarp_detector() {
    RabinKarpDetector rkd(2, 4);

    // Repeated pattern [100, 200] twice
    std::vector<double> amounts = {100, 200, 100, 200, 300, 400};
    int cnt = rkd.countRepeats(amounts, {100, 200});
    assert(cnt >= 1);

    // Score should be > 0 for repeated pattern
    double score = rkd.repetitionScore(amounts);
    assert(score > 0.0);

    // No repetition
    std::vector<double> flat = {10, 20, 30, 40, 50};
    score = rkd.repetitionScore(flat);
    assert(approx(score, 0.0));

    std::cout << "  PASS rabinkarp_detector\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Complete Player Lifecycle
// ═════════════════════════════════════════════════════════════════════

static void test_player_lifecycle() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    // Register
    assert(app.registerPlayer("p1", "Alice", "Alice", 1000.0, "", 18, "player"));
    assert(app.playerExists("p1"));
    assert(approx(app.getBalance("p1"), 1000.0));

    // Duplicate registration fails
    assert(!app.registerPlayer("p1", "Alice Again", "Alice Again", 500.0, "", 18, "player"));

    // Non-existent player
    assert(!app.playerExists("p_nonexistent"));
    assert(approx(app.getBalance("p_nonexistent"), -1.0));

    // Deposit
    assert(app.depositFunds("p1", 500.0));
    assert(approx(app.getBalance("p1"), 1500.0));

    // Withdraw
    assert(app.withdrawFunds("p1", 200.0));
    assert(approx(app.getBalance("p1"), 1300.0));

    // Over-withdraw fails
    assert(!app.withdrawFunds("p1", 999999.0));
    assert(approx(app.getBalance("p1"), 1300.0));

    cleanup(dir);
    std::cout << "  PASS player_lifecycle\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Complete Betting Cycle
// ═════════════════════════════════════════════════════════════════════

static void test_betting_cycle() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 10000.0, "", 18, "player");

    // Play a spin
    auto result = app.playSpin("p1", 100.0);
    assert(result.success);
    assert(result.playerId == "p1");
    assert(approx(result.betAmount, 100.0));
    assert(!result.symbols.empty());
    assert(result.balanceBefore == 10000.0);
    assert(result.balanceAfter <= result.balanceBefore - 100.0 + result.payout);
    assert(result.fraudScore >= 0.0);

    // Balance changed
    double bal = app.getBalance("p1");
    assert(!approx(bal, 10000.0));  // must have changed

    // Multiple spins
    for (int i = 0; i < 10; ++i) {
        result = app.playSpin("p1", 50.0);
        assert(result.success);
    }

    // Player's games logged
    auto games = app.getGameLogger().getPlayerHistory("p1");
    assert(games.size() == 11);

    cleanup(dir);
    std::cout << "  PASS betting_cycle\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Win / Loss Processing
// ═════════════════════════════════════════════════════════════════════

static void test_win_loss_processing() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 100000.0, "", 18, "player");

    int wins = 0;
    int losses = 0;
    for (int i = 0; i < 100; ++i) {
        auto result = app.playSpin("p1", 10.0);
        assert(result.success);
        if (result.isWin) {
            ++wins;
            assert(result.payout > 0.0);
            assert(result.balanceAfter > result.balanceBefore - result.betAmount);
        } else {
            ++losses;
            assert(approx(result.payout, 0.0));
            assert(approx(result.balanceAfter,
                          result.balanceBefore - result.betAmount));
        }
    }
    assert(wins + losses == 100);
    assert(wins > 0);   // statistically almost certain at 100 spins
    assert(losses > 0);

    cleanup(dir);
    std::cout << "  PASS win_loss_processing (" << wins << "W / " << losses << "L)\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Data Persistence
// ═════════════════════════════════════════════════════════════════════

static void test_data_persistence() {
    auto dir = tempDir();
    {
        ApplicationController app;
        app.initialise(dir);

        app.registerPlayer("p1", "Alice", "Alice", 5000.0, "", 18, "player");
        app.registerPlayer("p2", "Bob", "Bob", 3000.0, "", 18, "player");

        for (int i = 0; i < 5; ++i) {
            app.playSpin("p1", 100.0);
        }
        app.playSpin("p2", 50.0);

        assert(app.saveSystem());
    }
    // Destroyed — data should be on disk

    {
        ApplicationController app;
        app.initialise(dir);
        assert(app.loadSystem());

        // Players restored (balance after spins may differ from 5000)
        assert(app.playerExists("p1"));
        assert(app.playerExists("p2"));
        double bal = app.getBalance("p1");
        assert(bal >= 0);  // should be a valid loaded balance

        // Games restored
        auto games = app.getGameLogger().getAll();
        assert(games.size() >= 6);
    }

    cleanup(dir);
    std::cout << "  PASS data_persistence\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Leaderboard Integration
// ═════════════════════════════════════════════════════════════════════

static void test_leaderboard_integration() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 1000.0, "", 18, "player");
    app.registerPlayer("p2", "Bob", "Bob", 2000.0, "", 18, "player");
    app.registerPlayer("p3", "Charlie", "Charlie", 3000.0, "", 18, "player");

    // Top by balance
    auto top = app.getTopByBalance(2);
    assert(top.size() == 2);
    assert(top[0].getplayerid() == "p3");  // 3000
    assert(top[1].getplayerid() == "p2");  // 2000

    // After deposit, order may change
    app.depositFunds("p1", 3000.0);  // p1 now 4000
    top = app.getTopByBalance(3);
    assert(top[0].getplayerid() == "p1");  // 4000

    // Play some spins to affect other leaderboard categories
    app.depositFunds("p1", 100000.0);
    for (int i = 0; i < 20; ++i) {
        app.playSpin("p1", 100.0);
    }

    auto byGames = app.getTopByGamesPlayed(1);
    assert(!byGames.empty());

    cleanup(dir);
    std::cout << "  PASS leaderboard_integration\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Fraud Detection Integration
// ═════════════════════════════════════════════════════════════════════

static void test_fraud_detection_integration() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 100000.0, "", 18, "player");

    // Normal play should have low fraud score
    for (int i = 0; i < 20; ++i) {
        app.playSpin("p1", 100.0);
    }
    double score = app.runFraudCheck("p1");
    // Normal play should be low risk
    assert(score >= 0.0 && score <= 0.5);

    auto stats = app.getFraudStats();
    assert(stats.totalPlayersScanned >= 1);

    // Run on non-existent player (should not crash)
    score = app.runFraudCheck("nonexistent");
    assert(score >= 0.0);

    cleanup(dir);
    std::cout << "  PASS fraud_detection_integration\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Search Integration
// ═════════════════════════════════════════════════════════════════════

static void test_search_integration() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 1000.0, "", 18, "player");
    app.registerPlayer("p2", "Bob", "Bob", 500.0, "", 18, "player");
    app.registerPlayer("p3", "Alberto", "Alberto", 2000.0, "", 18, "player");

    // Player search by ID
    const player* p = app.searchPlayerById("p1");
    assert(p != nullptr);
    assert(p->getplayerid() == "p1");

    p = app.searchPlayerById("nonexistent");
    assert(p == nullptr);

    // Player search by name prefix
    auto alice = app.searchPlayerByName("Alice");
    assert(alice.size() == 1);

    auto al = app.searchPlayerByName("Al");
    assert(al.size() == 2);  // Alice, Alberto

    // Play games for search
    app.depositFunds("p1", 100000.0);
    for (int i = 0; i < 5; ++i) {
        app.playSpin("p1", 100.0);
    }

    // Game search
    auto g = app.searchGameById(0);
    assert(g.getplayerid() == "p1");

    bool threw = false;
    try { app.searchGameById(9999); }
    catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    // Symbol pattern search (API smoke test)
    auto symResults = app.searchSymbolPattern({"CHERRY"});
    // Just verify no crash — results depend on random spin outcomes

    cleanup(dir);
    std::cout << "  PASS search_integration\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Error Handling
// ═════════════════════════════════════════════════════════════════════

static void test_error_handling() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    // Spin with non-existent player
    auto result = app.playSpin("nonexistent", 100.0);
    assert(!result.success);
    assert(!result.errorMessage.empty());

    // Spin with insufficient funds
    app.registerPlayer("p1", "Poor Alice", "Poor Alice", 10.0, "", 18, "player");
    result = app.playSpin("p1", 100.0);
    assert(!result.success);
    assert(!result.errorMessage.empty());

    // Withdraw with insufficient funds
    assert(!app.withdrawFunds("p1", 9999.0));

    // Operations on non-existent player
    assert(!app.depositFunds("nobody", 100.0));
    assert(!app.withdrawFunds("nobody", 100.0));

    // Search for non-existent records
    assert(app.searchPlayerById("nobody") == nullptr);

    // Report for non-existent player (should not crash)
    auto report = app.generatePlayerReport("nobody", 10);
    assert(report.getplayerid() == "nobody");

    cleanup(dir);
    std::cout << "  PASS error_handling\n";
}

// ═════════════════════════════════════════════════════════════════════
//  Performance Report
// ═════════════════════════════════════════════════════════════════════

static void test_performance_report() {
    auto dir = tempDir();
    ApplicationController app;
    app.initialise(dir);

    app.registerPlayer("p1", "Alice", "Alice", 100000.0, "", 18, "player");
    for (int i = 0; i < 50; ++i) {
        app.playSpin("p1", 100.0);
    }

    auto report = app.getPerformanceReport();
    assert(report.totalSpins == 50);
    assert(report.houseEdge >= -10.0 && report.houseEdge <= 10.0);
    assert(report.overallWinRate >= 0.0 && report.overallWinRate <= 100.0);

    cleanup(dir);
    std::cout << "  PASS performance_report\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "FraudDetector sub-component tests:\n";
    test_circular_buffer();
    test_sliding_window();
    test_zscore_detector();
    test_rabinkarp_detector();

    std::cout << "\nSystem integration tests:\n";
    test_player_lifecycle();
    test_betting_cycle();
    test_win_loss_processing();
    test_data_persistence();
    test_leaderboard_integration();
    test_fraud_detection_integration();
    test_search_integration();
    test_error_handling();
    test_performance_report();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
