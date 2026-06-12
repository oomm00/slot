#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "ApplicationController.h"
#include "Leaderboard.h"
#include "PlayerManager.h"
#include "SlotMachine.h"
#include "BettingEngine.h"
#include "GameLogger.h"
#include "SearchEngine.h"
#include "FraudDetector.h"
#include "ProbabilityAnalyzer.h"

namespace fs = std::filesystem;
namespace chr = std::chrono;

using clock_type = chr::steady_clock;
using us = chr::microseconds;
using ms = chr::milliseconds;

static std::string g_dataDir;

// ── Helpers ──────────────────────────────────────────────────────────

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = clock_type::now().time_since_epoch().count();
    std::string dir = std::string(tmp) + "/bench_" + std::to_string(now);
    fs::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    fs::remove_all(dir);
}

struct BenchResult {
    std::string name;
    int64_t us; // total microseconds
    int64_t count;
};

static std::vector<BenchResult> g_results;

static void report(const std::string& name, int64_t us, int64_t count) {
    g_results.push_back({name, us, count});
    double sec = us / 1e6;
    double perSec = count / sec;
    std::cout << "  " << name << ": " << count << " ops in "
              << us / 1000 << " ms  (" << (int64_t)perSec << " ops/sec)\n";
}

// ── Player registration benchmark ────────────────────────────────────

static void bench_player_registration() {
    ApplicationController app;
    app.initialise(g_dataDir);

    const int N = 1000;
    auto start = clock_type::now();

    for (int i = 0; i < N; ++i) {
        std::string id = "p" + std::to_string(i);
        std::string name = "Player_" + std::to_string(i);
        app.registerPlayer(id, name, name, 1000.0, "", 18, "player");
    }

    auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
    report("Player registration", elapsed, N);
}

// ── Spin benchmark ───────────────────────────────────────────────────

static void bench_spins() {
    ApplicationController app;
    app.initialise(g_dataDir);

    // Pre-register 10 players
    for (int i = 0; i < 10; ++i) {
        app.registerPlayer("s" + std::to_string(i), "Spinner" + std::to_string(i), "Spinner" + std::to_string(i), 100000.0, "", 18, "player");
    }

    const int N = 1000;
    auto start = clock_type::now();

    for (int i = 0; i < N; ++i) {
        std::string pid = "s" + std::to_string(i % 10);
        app.playSpin(pid, 10.0 + (i % 10));
    }

    auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
    report("Spins", elapsed, N);
}

// ── Search benchmarks ────────────────────────────────────────────────

static void bench_searches() {
    ApplicationController app;
    app.initialise(g_dataDir);

    // Register players and do spins to populate search indices
    const int NUM_PLAYERS = 500;
    const int SPINS_PER = 10; // 5K total records
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        std::string pid = "q" + std::to_string(i);
        app.registerPlayer(pid, "QueryPlayer" + std::to_string(i), "QueryPlayer" + std::to_string(i), 10000.0, "", 18, "player");
    }
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        std::string pid = "q" + std::to_string(i);
        for (int j = 0; j < SPINS_PER; ++j) {
            app.playSpin(pid, 10.0);
        }
    }

    auto& se = app.getSearchEngine();
    se.rebuildIndices();

    // Binary search (player ID) — 10000 lookups
    {
        const int N = 10000;
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            std::string pid = "q" + std::to_string(i);
            volatile const auto* p = se.searchPlayerById(pid);
            (void)p;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("BinarySearch (player by ID)", elapsed, N);
    }

    // Binary search (game ID) — 10000 lookups
    {
        const int N = 10000;
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            try {
                volatile auto g = se.searchGameById(i);
                (void)g;
            } catch (...) {}
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("BinarySearch (game by ID)", elapsed, N);
    }

    // KMP symbol pattern search — 1000 searches
    {
        const int N = 1000;
        std::vector<std::string> pattern = {"SEVEN", "SEVEN"};
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            volatile auto results = se.searchSymbolPattern(pattern);
            (void)results;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("KMP (symbol pattern)", elapsed, N);
    }

    // Rabin-Karp bet pattern search — 1000 searches
    {
        const int N = 1000;
        std::vector<double> pattern = {10.0, 10.0, 10.0};
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            volatile auto results = se.searchBetPattern(pattern);
            (void)results;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("RabinKarp (bet pattern)", elapsed, N);
    }

    // Name prefix search (KMP-based linear scan) — 1000 queries
    {
        const int N = 1000;
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            volatile auto results = se.searchPlayerByName("QueryPlayer");
            (void)results;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Name prefix search", elapsed, N);
    }

    // Date range search — 100 queries
    {
        const int N = 100;
        auto start = clock_type::now();
        for (int i = 0; i < N; ++i) {
            volatile auto results = se.searchGamesByDateRange("2024-01-01", "2030-12-31");
            (void)results;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Date range search", elapsed, N);
    }
}

// ── Fraud detection benchmark ────────────────────────────────────────

static void bench_fraud_detection() {
    ApplicationController app;
    app.initialise(g_dataDir);

    const int NUM_PLAYERS = 50;
    const int SPINS_PER = 20; // 1K total records
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        std::string pid = "f" + std::to_string(i);
        app.registerPlayer(pid, "FraudTarget" + std::to_string(i), "FraudTarget" + std::to_string(i), 100000.0, "", 18, "player");
    }
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        std::string pid = "f" + std::to_string(i);
        for (int j = 0; j < SPINS_PER; ++j) {
            app.playSpin(pid, 10.0);
        }
    }

    // Run fraud check on all players
    auto start = clock_type::now();
    for (int i = 0; i < NUM_PLAYERS; ++i) {
        std::string pid = "f" + std::to_string(i);
        volatile double score = app.runFraudCheck(pid);
        (void)score;
    }
    auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
    report("Fraud detection (per player)", elapsed, NUM_PLAYERS);

    // Fraud stats
    auto start2 = clock_type::now();
    const int N2 = 1000;
    for (int i = 0; i < N2; ++i) {
        volatile auto stats = app.getFraudStats();
        (void)stats;
    }
    auto elapsed2 = chr::duration_cast<us>(clock_type::now() - start2).count();
    report("Fraud stats query", elapsed2, N2);
}

// ── Leaderboard benchmark ────────────────────────────────────────────

static void bench_leaderboard() {
    ApplicationController app;
    app.initialise(g_dataDir);

    const int N = 5000;
    for (int i = 0; i < N; ++i) {
        std::string pid = "l" + std::to_string(i);
        app.registerPlayer(pid, "LBPlayer" + std::to_string(i), "LBPlayer" + std::to_string(i), (double)(i % 10000), "", 18, "player");
    }

    // Get leaderboard (default: top N by whatever order)
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getLeaderboard(10);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Leaderboard (top 10)", elapsed, Q);
    }

    // Top by balance
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getTopByBalance(50);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("TopByBalance (50)", elapsed, Q);
    }

    // Top by win rate
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getTopByWinRate(50);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("TopByWinRate (50)", elapsed, Q);
    }

    // Top by total winnings
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getTopByTotalWinnings(50);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("TopByTotalWinnings (50)", elapsed, Q);
    }

    // Top by games played
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getTopByGamesPlayed(50);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("TopByGamesPlayed (50)", elapsed, Q);
    }

    // Top by biggest win
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            volatile auto lb = app.getTopByBiggestWin(50);
            (void)lb;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("TopByBiggestWin (50)", elapsed, Q);
    }
}

// ── Probability analysis benchmark ───────────────────────────────────

static void bench_probability_analysis() {
    ApplicationController app;
    app.initialise(g_dataDir);

    const int N = 20;
    for (int i = 0; i < N; ++i) {
        std::string pid = "a" + std::to_string(i);
        app.registerPlayer(pid, "AnalyticsPlayer" + std::to_string(i), "AnalyticsPlayer" + std::to_string(i), 100000.0, "", 18, "player");
    }
    for (int i = 0; i < N; ++i) {
        std::string pid = "a" + std::to_string(i);
        for (int j = 0; j < 20; ++j) {
            app.playSpin(pid, 10.0);
        }
    }

    // Strategy report
    {
        auto start = clock_type::now();
        const int Q = 100;
        for (int i = 0; i < Q; ++i) {
            std::string pid = "a" + std::to_string(i % N);
            volatile auto report = app.generatePlayerReport(pid, 100);
            (void)report;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Strategy report (DP/MC/Gambler)", elapsed, Q);
    }

    // System performance report
    {
        auto start = clock_type::now();
        const int Q = 1000;
        for (int i = 0; i < Q; ++i) {
            volatile auto pr = app.getPerformanceReport();
            (void)pr;
        }
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Performance report", elapsed, Q);
    }
}

// ── Persistence benchmark ────────────────────────────────────────────

static void bench_persistence() {
    std::string dir = tempDir();

    // Create data and save
    {
        ApplicationController app;
        app.initialise(dir);

        const int N = 1000;
        for (int i = 0; i < N; ++i) {
            std::string pid = "v" + std::to_string(i);
            app.registerPlayer(pid, "SavePlayer" + std::to_string(i), "SavePlayer" + std::to_string(i), 1000.0, "", 18, "player");
        }
        for (int i = 0; i < N; ++i) {
            std::string pid = "v" + std::to_string(i);
            app.playSpin(pid, 10.0);
        }

        auto start = clock_type::now();
        app.saveSystem();
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Save (1K players + spins)", elapsed, 1);
    }

    // Load
    {
        ApplicationController app;
        app.initialise(dir);

        auto start = clock_type::now();
        app.loadSystem();
        auto elapsed = chr::duration_cast<us>(clock_type::now() - start).count();
        report("Load (1K players + spins)", elapsed, 1);
    }

    cleanup(dir);
}

// ── Main ─────────────────────────────────────────────────────────────

int main() {
    g_dataDir = tempDir();
    std::cout << "Benchmark data dir: " << g_dataDir << "\n\n";

    std::cout << "=== Player Management ===\n";
    bench_player_registration();

    std::cout << "\n=== Slot Machine ===\n";
    bench_spins();

    std::cout << "\n=== Search Engine ===\n";
    bench_searches();

    std::cout << "\n=== Fraud Detection ===\n";
    bench_fraud_detection();

    std::cout << "\n=== Leaderboard ===\n";
    bench_leaderboard();

    std::cout << "\n=== Probability Analysis ===\n";
    bench_probability_analysis();

    std::cout << "\n=== Persistence ===\n";
    bench_persistence();

    // Summary
    std::cout << "\n=== SUMMARY ===\n";
    std::cout << "Benchmark                        |   Count  | Time (ms) |  Ops/sec\n";
    std::cout << "---------------------------------+----------+-----------+----------\n";
    for (auto& r : g_results) {
        char buf[256];
        double sec = r.us / 1e6;
        int64_t ops = (int64_t)(r.count / sec);
        std::string label = r.name;
        while (label.size() < 32) label += ' ';
        std::cout << label << " | " << std::right;
        std::cout.width(8);
        std::cout << r.count << " | ";
        std::cout.width(9);
        std::cout << (r.us / 1000) << " | ";
        std::cout << ops << "\n";
    }

    // Analysis
    std::cout << "\n=== ANALYSIS ===\n";

    // Leaderboard bottleneck
    auto lbResult = std::find_if(g_results.begin(), g_results.end(),
        [](auto& r) { return r.name == "Leaderboard (top 10)"; });
    if (lbResult != g_results.end() && lbResult->us > 0) {
        double perQuery = (double)lbResult->us / lbResult->count;
        std::cout << "! Leaderboard queries are slow (" << (int)perQuery
                  << " us/query) because refreshLeaderboard() copies ALL players\n"
                  << "  and rebuilds the entire leaderboard on every query.\n"
                  << "  Estimated at 100K players: ~" << (int)(perQuery * 100) << " ms/query\n";
    }

    // Registration O(n^2) bottleneck
    auto regResult = std::find_if(g_results.begin(), g_results.end(),
        [](auto& r) { return r.name == "Player registration"; });
    if (regResult != g_results.end() && regResult->count > 0) {
        double perReg = (double)regResult->us / regResult->count;
        std::cout << "! Registration: " << (int)perReg << " us/player (avg for "
                  << regResult->count << " players)\n"
                  << "  Scales O(n^2) due to rebuildIndices() + refreshLeaderboard() per registration.\n"
                  << "  Estimated for 100K players: ~"
                  << (int)(perReg * 100000 * 100000 / (regResult->count * regResult->count) / 1000)
                  << " seconds total\n";
    }

    // KMP/RabinKarp scanning bottleneck
    auto kmpResult = std::find_if(g_results.begin(), g_results.end(),
        [](auto& r) { return r.name == "KMP (symbol pattern)"; });
    if (kmpResult != g_results.end() && kmpResult->us > 0) {
        double perKmp = (double)kmpResult->us / kmpResult->count;
        std::cout << "! KMP/RabinKarp: " << (int)perKmp << " us/query (linear scan of ALL records)\n"
                  << "  Estimated at 1M records: ~" << (int)(perKmp * 200) << " us/query\n";
    }

    std::cout << "\nAll benchmarks completed.\n";
    cleanup(g_dataDir);
    return 0;
}
