#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "BetType.h"
#include "BettingEngine.h"
#include "CSVStorage.h"
#include "GameLogger.h"
#include "GamblerRuin.h"
#include "MarkovChain.h"
#include "Player.h"
#include "PlayerManager.h"
#include "ProbabilityAnalyzer.h"
#include "ProbabilityDP.h"
#include "SlotMachine.h"

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::string dir = std::string(tmp) + "/probanalyzer_test/" + std::to_string(now);
    std::filesystem::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    std::filesystem::remove_all(dir);
}

static bool approx(double a, double b, double eps = 0.001) {
    return std::abs(a - b) < eps;
}

// ─── ProbabilityDP tests ────────────────────────────────────────────

static void test_dp_expected_profit_positive() {
    // p=0.5, mult=3.0 → edge = 0.5*3-1 = 0.5 per bet
    ProbabilityDP dp(0.5, 3.0);
    double profit = dp.expectedProfitAfter(100, 10.0);
    assert(approx(profit, 500.0));  // 100 * 10 * 0.5
    std::cout << "  PASS dp_expected_profit_positive\n";
}

static void test_dp_expected_profit_negative() {
    // p=0.2, mult=0.98 → edge = 0.2*0.98-1 = -0.804
    ProbabilityDP dp(0.2, 0.98);
    double profit = dp.expectedProfitAfter(10, 5.0);
    double expected = 10 * 5.0 * (0.2 * 0.98 - 1.0);
    assert(approx(profit, expected));
    std::cout << "  PASS dp_expected_profit_negative\n";
}

static void test_dp_expected_profit_zero_bets() {
    ProbabilityDP dp(0.5, 3.0);
    assert(approx(dp.expectedProfitAfter(0, 100.0), 0.0));
    std::cout << "  PASS dp_expected_profit_zero_bets\n";
}

static void test_dp_distribution_sums_to_one() {
    ProbabilityDP dp(0.3, 2.0);
    auto dist = dp.profitDistribution(50);
    double sum = 0.0;
    for (double p : dist) sum += p;
    assert(approx(sum, 1.0));
    std::cout << "  PASS dp_distribution_sums_to_one\n";
}

static void test_dp_distribution_length() {
    ProbabilityDP dp(0.5, 2.0);
    auto dist = dp.profitDistribution(20);
    assert(dist.size() == 21);
    std::cout << "  PASS dp_distribution_length\n";
}

static void test_dp_prob_of_profit() {
    // p=1.0, mult=2.0 → always wins, profit always positive
    ProbabilityDP dp(1.0, 2.0);
    double prob = dp.probabilityOfProfitAfter(10, 1.0);
    assert(approx(prob, 1.0));
    std::cout << "  PASS dp_prob_of_profit_certain\n";
}

static void test_dp_prob_of_profit_impossible() {
    // p=0.0 → never wins, profit never positive
    ProbabilityDP dp(0.0, 2.0);
    double prob = dp.probabilityOfProfitAfter(10, 1.0);
    assert(approx(prob, 0.0));
    std::cout << "  PASS dp_prob_of_profit_impossible\n";
}

static void test_dp_kelly_zero_edge() {
    ProbabilityDP dp(0.5, 2.0);  // edge = 0.0
    double kelly = dp.optimalBetSize(1000.0);
    assert(approx(kelly, 0.0));
    std::cout << "  PASS dp_kelly_zero_edge\n";
}

static void test_dp_kelly_positive() {
    // p=0.6, mult=2.0 → b=2, edge=0.2, full kelly = (p*b - q)/b
    // = (0.6*2 - 0.4)/2 = 0.8/2 = 0.4
    ProbabilityDP dp(0.6, 2.0);
    double kelly = dp.optimalBetSize(1000.0);
    assert(approx(kelly, 0.4));
    std::cout << "  PASS dp_kelly_positive\n";
}

// ─── MarkovChain tests ──────────────────────────────────────────────

static void test_mc_steady_state() {
    // a=0.2, b=0.5 → π₀ = (1-0.5)/(1-0.5+0.2) = 0.5/0.7 ≈ 0.7143
    // π₁ = 0.2/0.7 ≈ 0.2857
    MarkovChain mc(0.2, 0.5);
    double loss = mc.steadyStateLoss();
    double win  = mc.steadyStateWin();
    assert(approx(loss, 0.714286, 0.001));
    assert(approx(win,  0.285714, 0.001));
    assert(approx(loss + win, 1.0));
    std::cout << "  PASS mc_steady_state\n";
}

static void test_mc_always_win() {
    // a=1.0, b=1.0 → always in WIN state
    MarkovChain mc(1.0, 1.0);
    assert(approx(mc.steadyStateWin(), 1.0));
    assert(approx(mc.steadyStateLoss(), 0.0));
    std::cout << "  PASS mc_always_win\n";
}

static void test_mc_expected_streaks() {
    // a=0.25, b=0.75 → expected loss streak = 1/0.25 = 4, win streak = 1/0.25 = 4
    MarkovChain mc(0.25, 0.75);
    assert(approx(mc.expectedLossStreak(), 4.0));
    assert(approx(mc.expectedWinStreak(), 4.0));
    std::cout << "  PASS mc_expected_streaks\n";
}

static void test_mc_power_iteration_converges() {
    MarkovChain mc(0.3, 0.6);
    auto analytic = std::make_pair(mc.steadyStateLoss(), mc.steadyStateWin());
    auto iterated = mc.powerIteration(100);
    assert(approx(analytic.first, iterated.first, 0.0001));
    assert(approx(analytic.second, iterated.second, 0.0001));
    std::cout << "  PASS mc_power_iteration_converges\n";
}

static void test_mc_state_after() {
    // a=0.5, b=0.0 → from LOSS: P(LOSS)=0.5, P(WIN)=0.5 after 1 step
    // from WIN: P(LOSS)=1.0, P(WIN)=0.0 after 1 step
    MarkovChain mc(0.5, 0.0);
    auto fromLoss = mc.stateAfter(1, false);
    assert(approx(fromLoss.first, 0.5));
    assert(approx(fromLoss.second, 0.5));

    auto fromWin = mc.stateAfter(1, true);
    assert(approx(fromWin.first, 1.0));
    assert(approx(fromWin.second, 0.0));
    std::cout << "  PASS mc_state_after\n";
}

// ─── GamblerRuin tests ─────────────────────────────────────────────

static void test_ruin_certain_ruin() {
    // initial=0 → already ruined
    GamblerRuin gr(0.0, 100.0, 0.5, 2.0);
    assert(approx(gr.ruinProbability(), 1.0));
    assert(approx(gr.successProbability(), 0.0));
    std::cout << "  PASS ruin_certain_ruin\n";
}

static void test_ruin_certain_success() {
    // initial >= goal → already succeeded
    GamblerRuin gr(100.0, 50.0, 0.5, 2.0);
    assert(approx(gr.ruinProbability(), 0.0));
    assert(approx(gr.successProbability(), 1.0));
    std::cout << "  PASS ruin_certain_success\n";
}

static void test_ruin_fair_coin() {
    // Fair odds: p=q=0.5, ruin = 1 - initial/goal = 1 - 50/200 = 0.75
    GamblerRuin gr(50.0, 200.0, 0.5, 2.0);
    assert(approx(gr.ruinProbability(), 0.75));
    std::cout << "  PASS ruin_fair_coin\n";
}

static void test_ruin_unfair() {
    // p=0.4, q=0.6, r=1.5, initial=10, goal=30
    // ruin = (1.5^10 - 1.5^30)/(1 - 1.5^30)
    GamblerRuin gr(10.0, 30.0, 0.4, 2.0);
    double r = 0.6 / 0.4;
    double rI = std::pow(r, 10.0);
    double rG = std::pow(r, 30.0);
    double expected = (rI - rG) / (1.0 - rG);
    assert(approx(gr.ruinProbability(), expected));
    std::cout << "  PASS ruin_unfair\n";
}

static void test_ruin_expected_duration_fair() {
    // Fair: E[spins] = initial * (goal - initial) = 50 * 150 = 7500
    GamblerRuin gr(50.0, 200.0, 0.5, 2.0);
    assert(approx(gr.expectedDuration(), 7500.0));
    std::cout << "  PASS ruin_expected_duration_fair\n";
}

// ─── ProbabilityAnalyzer integration tests ──────────────────────────

static void test_analyzer_report_basic() {
    ProbabilityAnalyzer pa;
    auto report = pa.reportForPlayer("p1", 1000.0, 100);

    assert(report.getplayerid() == "p1");
    assert(approx(report.getcurrbal(), 1000.0));
    assert(report.getexpectreturn() < 0.0);  // house edge = negative EV
    assert(report.getrisk() >= 0.0 && report.getrisk() <= 1.0);
    assert(report.getconfidence() >= 0.0);
    assert(!report.getrecomstrategy().empty());
    assert(report.getrecomamt() > 0.0);

    std::cout << "  PASS analyzer_report_basic\n";
}

static void test_analyzer_report_strategy_negative_ev() {
    ProbabilityAnalyzer pa;
    auto report = pa.reportForPlayer("p2", 100.0, 10);
    // With negative EV, strategy should be "reduce_bet"
    if (report.getexpectreturn() <= 0.0) {
        assert(report.getrecomstrategy() == "reduce_bet");
    }
    std::cout << "  PASS analyzer_report_strategy_negative_ev\n";
}

static void test_analyzer_analyze_sessions() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    pm.addPlayer(player("p1", "Alice", "Alice", 1000.0, "", "player", 18));
    pm.addPlayer(player("p2", "Bob", "Bob", 500.0, "", "player", 18));

    ProbabilityAnalyzer pa;
    auto reports = pa.analyzeGameSessions(pm, 50);

    assert(reports.size() == 2);
    // Sorted by expectedReturn descending
    assert(reports[0].getexpectreturn() >= reports[1].getexpectreturn());

    cleanup(dir);
    std::cout << "  PASS analyzer_analyze_sessions\n";
}

static void test_analyzer_performance_report() {
    auto dir = tempDir();
    PlayerManager pm{CSVStorage(dir)};
    SlotMachine sm;
    GameLogger logger;
    BettingEngine engine(pm, sm, logger);

    pm.addPlayer(player("p1", "Alice", "Alice", 10000.0, "", "player", 18));
    for (int i = 0; i < 20; ++i) {
        engine.processSpin("p1", 100.0, BetType::ANY_PAIR, {});
    }

    ProbabilityAnalyzer pa;
    auto stats = engine.getStats();
    auto report = pa.performanceReport(stats);

    assert(report.totalSpins == 20);
    assert(report.houseEdge >= -10.0 && report.houseEdge <= 10.0);
    assert(report.overallWinRate >= 0.0 && report.overallWinRate <= 100.0);
    assert(report.avgStreakLength > 0.0);
    assert(report.avgRuinRisk >= 0.0);
    assert(report.totalPlayers == 0);  // not tracked

    cleanup(dir);
    std::cout << "  PASS analyzer_performance_report\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "ProbabilityDP tests:\n";
    test_dp_expected_profit_positive();
    test_dp_expected_profit_negative();
    test_dp_expected_profit_zero_bets();
    test_dp_distribution_sums_to_one();
    test_dp_distribution_length();
    test_dp_prob_of_profit();
    test_dp_prob_of_profit_impossible();
    test_dp_kelly_zero_edge();
    test_dp_kelly_positive();

    std::cout << "\nMarkovChain tests:\n";
    test_mc_steady_state();
    test_mc_always_win();
    test_mc_expected_streaks();
    test_mc_power_iteration_converges();
    test_mc_state_after();

    std::cout << "\nGamblerRuin tests:\n";
    test_ruin_certain_ruin();
    test_ruin_certain_success();
    test_ruin_fair_coin();
    test_ruin_unfair();
    test_ruin_expected_duration_fair();

    std::cout << "\nProbabilityAnalyzer integration tests:\n";
    test_analyzer_report_basic();
    test_analyzer_report_strategy_negative_ev();
    test_analyzer_analyze_sessions();
    test_analyzer_performance_report();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
