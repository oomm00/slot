#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Leaderboard.h"
#include "Player.h"

static bool approx(double a, double b, double eps = 0.0001) {
    return std::abs(a - b) < eps;
}

// ─── Test helpers ─────────────────────────────────────────────────────

/// Build a known set of 6 players with controlled stats.
static std::vector<player> makeTestPlayers() {
    std::vector<player> players;

    // p1 – Alice:  2 wins / 3 games, twon=8000, bwin=5000, bal=17000
    player p1("p1", "Alice", "Alice", 10000.0, "", "player", 18);
    p1.recordwin(3000.0);
    p1.recordloss(1000.0);
    p1.recordwin(5000.0);
    p1.addwager(5000.0);
    players.push_back(p1);

    // p2 – Bob: 3/3 wins (100%), twon=6500, bwin=3000, bal=11500
    player p2("p2", "Bob", "Bob", 5000.0, "", "player", 18);
    p2.recordwin(2000.0);
    p2.recordwin(1500.0);
    p2.recordwin(3000.0);
    p2.addwager(3000.0);
    players.push_back(p2);

    // p3 – Charlie: 0 games, bal=20000 (highest balance)
    player p3("p3", "Charlie", "Charlie", 20000.0, "", "player", 18);
    players.push_back(p3);

    // p4 – Diana: 1/1 win  (100%), twon=10000, bwin=10000, bal=18000
    player p4("p4", "Diana", "Diana", 8000.0, "", "player", 18);
    p4.recordwin(10000.0);
    p4.addwager(8000.0);
    players.push_back(p4);

    // p5 – Eve: 1/4 wins (25%), 4 games (most), bwin=500, bal=1000
    player p5("p5", "Eve", "Eve", 3000.0, "", "player", 18);
    p5.recordloss(500.0);
    p5.recordloss(1000.0);
    p5.recordloss(1000.0);
    p5.recordwin(500.0);
    p5.addwager(500.0);
    players.push_back(p5);

    // p6 – Frank: 2/3 wins (tie with Alice), bwin=2000, bal=7500
    player p6("p6", "Frank", "Frank", 5000.0, "", "player", 18);
    p6.recordwin(1000.0);
    p6.recordloss(500.0);
    p6.recordwin(2000.0);
    p6.addwager(2000.0);
    players.push_back(p6);

    return players;
}

// ─── 1. Empty leaderboard ────────────────────────────────────────────

static void test_empty() {
    Leaderboard lb;
    assert(lb.size() == 0);

    assert(lb.getTopPlayers(5).empty());
    assert(lb.getTopByBalance(5).empty());
    assert(lb.getTopByWinRate(5).empty());
    assert(lb.getTopByTotalWinnings(5).empty());
    assert(lb.getTopByGamesPlayed(5).empty());
    assert(lb.getTopByBiggestWin(5).empty());

    assert(lb.getPlayerRank("nonexistent") == 0);
    assert(approx(lb.getPercentile("nonexistent"), -1.0));

    bool threw = false;
    try { lb.getRankedPlayer(1); } catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    std::cout << "  PASS empty\n";
}

// ─── 2. Top by balance ───────────────────────────────────────────────

static void test_top_by_balance() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopByBalance(6);
    assert(top.size() == 6);
    assert(top[0].getplayerid() == "p3");  // 20000
    assert(top[1].getplayerid() == "p4");  // 18000
    assert(top[2].getplayerid() == "p1");  // 17000
    assert(top[3].getplayerid() == "p2");  // 11500
    assert(top[4].getplayerid() == "p6");  // 7500
    assert(top[5].getplayerid() == "p5");  // 1000

    std::cout << "  PASS top_by_balance\n";
}

// ─── 3. Top by win rate ──────────────────────────────────────────────

static void test_top_by_win_rate() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopByWinRate(6);
    assert(top.size() == 6);
    // p2: 3/3=1.0  >  p4: 1/1=1.0 (tie → more games wins)
    assert(top[0].getplayerid() == "p2");
    assert(top[1].getplayerid() == "p4");
    // p1: 2/3=0.667  >  p6: 2/3=0.667 (tie → "p1" < "p6")
    assert(top[2].getplayerid() == "p1");
    assert(top[3].getplayerid() == "p6");
    assert(top[4].getplayerid() == "p5");  // 0.25
    assert(top[5].getplayerid() == "p3");  // 0.0

    std::cout << "  PASS top_by_win_rate\n";
}

// ─── 4. Top by total winnings ────────────────────────────────────────

static void test_top_by_total_winnings() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopByTotalWinnings(6);
    assert(top.size() == 6);
    assert(top[0].getplayerid() == "p4");  // 10000
    assert(top[1].getplayerid() == "p1");  // 8000
    assert(top[2].getplayerid() == "p2");  // 6500
    assert(top[3].getplayerid() == "p6");  // 3000
    assert(top[4].getplayerid() == "p5");  // 500
    assert(top[5].getplayerid() == "p3");  // 0

    std::cout << "  PASS top_by_total_winnings\n";
}

// ─── 5. Top by games played ──────────────────────────────────────────

static void test_top_by_games_played() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopByGamesPlayed(6);
    assert(top.size() == 6);
    assert(top[0].getplayerid() == "p5");  // 4
    // p1/p2/p6 all have 3 games; tiebreak → ID order
    assert(top[1].getplayerid() == "p1");  // "p1" < "p2" < "p6"
    assert(top[2].getplayerid() == "p2");
    assert(top[3].getplayerid() == "p6");
    assert(top[4].getplayerid() == "p4");  // 1
    assert(top[5].getplayerid() == "p3");  // 0

    std::cout << "  PASS top_by_games_played\n";
}

// ─── 6. Top by biggest win ────────────────────────────────────────────

static void test_top_by_biggest_win() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopByBiggestWin(6);
    assert(top.size() == 6);
    assert(top[0].getplayerid() == "p4");  // 10000
    assert(top[1].getplayerid() == "p1");  // 5000
    assert(top[2].getplayerid() == "p2");  // 3000
    assert(top[3].getplayerid() == "p6");  // 2000
    assert(top[4].getplayerid() == "p5");  // 500
    assert(top[5].getplayerid() == "p3");  // 0

    std::cout << "  PASS top_by_biggest_win\n";
}

// ─── 7. getTopPlayers (global = balance) ──────────────────────────────

static void test_top_players() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopPlayers(3);
    assert(top.size() == 3);
    assert(top[0].getplayerid() == "p3");
    assert(top[1].getplayerid() == "p4");
    assert(top[2].getplayerid() == "p1");

    std::cout << "  PASS top_players\n";
}

// ─── 8. getRankedPlayer ──────────────────────────────────────────────

static void test_get_ranked_player() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    // Global ranking is by balance
    player r1 = lb.getRankedPlayer(1);
    assert(r1.getplayerid() == "p3");  // highest balance

    player r3 = lb.getRankedPlayer(3);
    assert(r3.getplayerid() == "p1");

    player r6 = lb.getRankedPlayer(6);
    assert(r6.getplayerid() == "p5");  // lowest balance

    // Out-of-range
    bool threw = false;
    try { lb.getRankedPlayer(0); }  catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    threw = false;
    try { lb.getRankedPlayer(7); }  catch (const std::out_of_range&) { threw = true; }
    assert(threw);

    std::cout << "  PASS get_ranked_player\n";
}

// ─── 9. getPlayerRank ────────────────────────────────────────────────

static void test_get_player_rank() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    assert(lb.getPlayerRank("p3") == 1);  // highest balance
    assert(lb.getPlayerRank("p4") == 2);
    assert(lb.getPlayerRank("p1") == 3);
    assert(lb.getPlayerRank("p2") == 4);
    assert(lb.getPlayerRank("p6") == 5);
    assert(lb.getPlayerRank("p5") == 6);

    // Non-existent
    assert(lb.getPlayerRank("nobody") == 0);

    std::cout << "  PASS get_player_rank\n";
}

// ─── 10. getPercentile ────────────────────────────────────────────────

static void test_get_percentile() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    // Rank 1 → 100.0% (top of chart)
    assert(approx(lb.getPercentile("p3"), 100.0));

    // Rank 3 → (6-3)/(6-1) * 100 = 60.0%
    assert(approx(lb.getPercentile("p1"), 60.0));

    // Rank 6 → 0.0%
    assert(approx(lb.getPercentile("p5"), 0.0));

    // Unknown
    assert(approx(lb.getPercentile("nobody"), -1.0));

    std::cout << "  PASS get_percentile\n";
}

// ─── 11. Tie handling ────────────────────────────────────────────────

static void test_tie_handling() {
    // Already implicitly tested above (p1/p6 same win rate),
    // but let's verify explicitly by adding two players with *identical* stats.
    std::vector<player> players;

    player a("alpha", "Alpha", "Alpha", 5000.0, "", "player", 18);
    a.recordwin(1000.0);
    a.recordloss(500.0);
    a.recordwin(2000.0);
    // bal=7500, wr=2/3, twon=3000, gp=3, bwin=2000
    players.push_back(a);

    player b("beta", "Beta", "Beta", 5000.0, "", "player", 18);
    b.recordwin(1000.0);
    b.recordloss(500.0);
    b.recordwin(2000.0);
    // bal=7500, wr=2/3, twon=3000, gp=3, bwin=2000 — identical
    players.push_back(b);

    Leaderboard lb;
    lb.update(players);

    // By balance — both 7500 → tiebreak by ID
    auto top = lb.getTopByBalance(2);
    assert(top[0].getplayerid() == "alpha");  // "alpha" < "beta"
    assert(top[1].getplayerid() == "beta");

    // By win rate — both 66.7% → tiebreak by games played (both 3) → ID
    top = lb.getTopByWinRate(2);
    assert(top[0].getplayerid() == "alpha");
    assert(top[1].getplayerid() == "beta");

    std::cout << "  PASS tie_handling\n";
}

// ─── 12. Top-K with n > size ─────────────────────────────────────────

static void test_n_larger_than_size() {
    Leaderboard lb;
    lb.update(makeTestPlayers());

    auto top = lb.getTopPlayers(100);
    assert(top.size() == lb.size());

    std::cout << "  PASS n_larger_than_size\n";
}

// ─── 13. Update is idempotent ─────────────────────────────────────────

static void test_update_idempotent() {
    Leaderboard lb;
    auto players = makeTestPlayers();
    lb.update(players);
    lb.update(players);  // second update with same data

    auto top1 = lb.getTopByBalance(3);
    lb.update(players);  // third
    auto top2 = lb.getTopByBalance(3);

    assert(top1.size() == top2.size());
    for (size_t i = 0; i < top1.size(); ++i) {
        assert(top1[i].getplayerid() == top2[i].getplayerid());
    }

    std::cout << "  PASS update_idempotent\n";
}

// ─── 14. Large dataset performance ────────────────────────────────────

static void test_large_dataset() {
    Leaderboard lb;

    const int N = 10000;
    std::vector<player> players;
    players.reserve(N);

    for (int i = 0; i < N; ++i) {
        std::string id = "p" + std::to_string(i);
        double bal = static_cast<double>((i * 937) % 100000);
        player p(id, "Player" + std::to_string(i), "Player" + std::to_string(i), bal, "", "player", 18);
        // Give some stats for variety
        if (i % 3 == 0) p.recordwin(static_cast<double>(i * 10));
        if (i % 5 == 0) p.recordloss(static_cast<double>(i * 5));
        players.push_back(p);
    }

    auto start = std::chrono::steady_clock::now();
    lb.update(players);
    auto mid = std::chrono::steady_clock::now();

    auto top10 = lb.getTopByBalance(10);
    auto top100 = lb.getTopByWinRate(100);
    size_t rank = lb.getPlayerRank("p0");
    double pct = lb.getPercentile("p5000");
    player r1 = lb.getRankedPlayer(1);
    auto end = std::chrono::steady_clock::now();

    auto updateMs = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
    auto queryMs = std::chrono::duration_cast<std::chrono::microseconds>(end - mid).count();

    assert(top10.size() == 10);
    assert(top100.size() == 100);
    assert(rank > 0);
    assert(pct >= 0.0 && pct <= 100.0);
    assert(r1.getplayerid() == top10[0].getplayerid());

    std::cout << "  PASS large_dataset (update=" << updateMs
              << "ms, queries=" << queryMs << "us)\n";
}

// ─── 15. Rebuild after larger set ─────────────────────────────────────

static void test_rebuild() {
    Leaderboard lb;

    std::vector<player> p1;
    p1.push_back(player("a", "A", "A", 100.0, "", "player", 18));
    p1.push_back(player("b", "B", "B", 200.0, "", "player", 18));
    lb.update(p1);
    assert(lb.size() == 2);

    // Replace with entirely different set
    std::vector<player> p2;
    p2.push_back(player("x", "X", "X", 500.0, "", "player", 18));
    p2.push_back(player("y", "Y", "Y", 300.0, "", "player", 18));
    p2.push_back(player("z", "Z", "Z", 400.0, "", "player", 18));
    lb.update(p2);
    assert(lb.size() == 3);
    assert(lb.getPlayerRank("a") == 0);  // old player gone
    assert(lb.getPlayerRank("x") == 1);  // highest balance

    std::cout << "  PASS rebuild\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "Leaderboard tests:\n";

    test_empty();
    test_top_by_balance();
    test_top_by_win_rate();
    test_top_by_total_winnings();
    test_top_by_games_played();
    test_top_by_biggest_win();
    test_top_players();
    test_get_ranked_player();
    test_get_player_rank();
    test_get_percentile();
    test_tie_handling();
    test_n_larger_than_size();
    test_update_idempotent();
    test_large_dataset();
    test_rebuild();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
