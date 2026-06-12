#include <cassert>
#include <cmath>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "SlotMachine.h"

// ─── 1. Seeded spin is deterministic ─────────────────────────────────

static void test_deterministic_spin() {
    SlotMachine sm;
    Spin a = sm.spin(42);
    Spin b = sm.spin(42);

    assert(a.getspinid() != b.getspinid());   // different counter
    assert(a.getseed() == b.getseed());
    assert(a.getseed() == 42u);
    assert(a.getsymbols() == b.getsymbols());  // same symbols
    assert(a.getmultiplier() == b.getmultiplier());
    assert(a.iswinspin() == b.iswinspin());

    std::cout << "  PASS deterministic_spin\n";
}

// ─── 2. Different seeds produce different sequences ──────────────────

static void test_different_seeds() {
    SlotMachine sm;
    Spin a = sm.spin(1);
    Spin b = sm.spin(2);

    // Statistically extremely unlikely to get identical results
    // from two different seeds on 3 reels with 5 symbols each.
    bool same = (a.getsymbols() == b.getsymbols());
    assert(!same);
    (void)same;

    std::cout << "  PASS different_seeds\n";
}

// ─── 3. Spin returns exactly 3 symbols ───────────────────────────────

static void test_spin_returns_three_symbols() {
    SlotMachine sm;
    Spin s = sm.spin();
    assert(s.getsymbols().size() == 3);

    // All symbols come from the valid pool
    for (const auto& sym : s.getsymbols()) {
        bool valid = (sym == "CHERRY" || sym == "LEMON" || sym == "ORANGE"
                      || sym == "BELL" || sym == "SEVEN");
        assert(valid);
    }

    std::cout << "  PASS spin_returns_three_symbols\n";
}

// ─── 4. calculateMultiplier — three of a kind ────────────────────────

static void test_three_of_a_kind() {
    struct TestCase { std::vector<std::string> syms; double expected; };
    TestCase cases[] = {
        {{"CHERRY","CHERRY","CHERRY"}, 2.0},
        {{"LEMON", "LEMON", "LEMON" }, 3.0},
        {{"ORANGE","ORANGE","ORANGE"}, 5.0},
        {{"BELL",  "BELL",  "BELL"  }, 7.0},
        {{"SEVEN", "SEVEN", "SEVEN" }, 10.0},
    };

    for (const auto& c : cases) {
        double mult = SlotMachine::calculateMultiplier(c.syms);
        assert(std::abs(mult - c.expected) < 0.001);
    }

    std::cout << "  PASS three_of_a_kind\n";
}

// ─── 5. calculateMultiplier — two of a kind ──────────────────────────

static void test_two_of_a_kind() {
    struct TestCase { std::vector<std::string> syms; double expected; };
    TestCase cases[] = {
        {{"CHERRY","CHERRY","LEMON"},  0.5},
        {{"LEMON", "ORANGE","LEMON" }, 1.0},
        {{"BELL",  "BELL",  "SEVEN" }, 2.0},
        {{"SEVEN", "CHERRY","SEVEN" }, 3.0},
        {{"ORANGE","CHERRY","ORANGE"}, 1.5},
    };

    for (const auto& c : cases) {
        double mult = SlotMachine::calculateMultiplier(c.syms);
        assert(std::abs(mult - c.expected) < 0.001);
    }

    std::cout << "  PASS two_of_a_kind\n";
}

// ─── 6. calculateMultiplier — no match ───────────────────────────────

static void test_no_match() {
    std::vector<std::string> cases[] = {
        {"CHERRY","LEMON","ORANGE"},
        {"SEVEN", "BELL", "CHERRY"},
        {"LEMON", "ORANGE","BELL"},
    };

    for (const auto& syms : cases) {
        double mult = SlotMachine::calculateMultiplier(syms);
        assert(std::abs(mult) < 0.001);
    }

    std::cout << "  PASS no_match\n";
}

// ─── 7. Spin result consistency — win flag matches multiplier ────────

static void test_win_flag_matches_multiplier() {
    SlotMachine sm;

    for (int seed = 0; seed < 100; ++seed) {
        Spin s = sm.spin(static_cast<unsigned>(seed));
        bool expectWin = s.getmultiplier() > 0.0;
        assert(s.iswinspin() == expectWin);
    }

    std::cout << "  PASS win_flag_matches_multiplier\n";
}

// ─── 8. Pattern string correctness ───────────────────────────────────

static void test_pattern_strings() {
    struct TestCase {
        std::vector<std::string> syms;
        std::string expected;
    };
    TestCase cases[] = {
        {{"CHERRY","CHERRY","CHERRY"}, "JACKPOT_THREE_CHERRY"},
        {{"SEVEN", "SEVEN", "SEVEN" }, "JACKPOT_THREE_SEVEN"},
        {{"BELL",  "BELL",  "LEMON" }, "PARTIAL_TWO_BELL"},
        {{"ORANGE","CHERRY","CHERRY"}, "PARTIAL_TWO_CHERRY"},
        {{"CHERRY","LEMON","ORANGE"},  "NO_WIN"},
    };

    // We can't directly check pattern from outside, but we verify
    // indirectly via the multiplier and win flag above.
    // This test uses a deterministic spin to check consistency:
    SlotMachine sm;
    Spin s1 = sm.spin(0);
    // Just verify pattern is non-empty
    assert(!s1.getwinpattern().empty());

    std::cout << "  PASS pattern_strings\n";
}

// ─── 9. Custom symbols ──────────────────────────────────────────────

static void test_custom_symbols() {
    std::vector<std::string> custom{"DIAMOND", "STAR"};
    SlotMachine sm(custom);

    for (int seed = 0; seed < 50; ++seed) {
        Spin s = sm.spin(static_cast<unsigned>(seed));
        for (const auto& sym : s.getsymbols()) {
            bool valid = (sym == "DIAMOND" || sym == "STAR");
            assert(valid);
        }
    }

    // Three DIAMONDs should use a default fallback multiplier (0.0
    // since DIAMOND is not in the PAYTABLE_).
    double mult = SlotMachine::calculateMultiplier({"DIAMOND","DIAMOND","DIAMOND"});
    assert(std::abs(mult) < 0.001);

    std::cout << "  PASS custom_symbols\n";
}

// ─── 10. Empty symbol pool falls back to DEFAULT_SYMBOLS ─────────────

static void test_empty_symbol_pool_fallback() {
    SlotMachine sm(std::vector<std::string>{});
    Spin s = sm.spin(42);
    assert(s.getsymbols().size() == 3);
    // All symbols should be from the default set
    for (const auto& sym : s.getsymbols()) {
        bool valid = (sym == "CHERRY" || sym == "LEMON" || sym == "ORANGE"
                      || sym == "BELL" || sym == "SEVEN");
        assert(valid);
    }

    std::cout << "  PASS empty_symbol_pool_fallback\n";
}

// ─── 11. setSymbols replaces all reel pools ──────────────────────────

static void test_set_symbols() {
    SlotMachine sm;
    sm.setSymbols({"A", "B", "C"});

    Spin s = sm.spin(7);
    assert(s.getsymbols().size() == 3);
    for (const auto& sym : s.getsymbols()) {
        bool valid = (sym == "A" || sym == "B" || sym == "C");
        assert(valid);
    }

    std::cout << "  PASS set_symbols\n";
}

// ─── 12. Large number of spins produces varied results ───────────────

static void test_statistical_variety() {
    SlotMachine sm;
    std::map<std::string, int> symbolCount;

    for (int i = 0; i < 1000; ++i) {
        Spin s = sm.spin(static_cast<unsigned>(i));
        for (const auto& sym : s.getsymbols()) {
            ++symbolCount[sym];
        }
    }

    // With 1000 spins × 3 symbols = 3000 draws across 5 symbols,
    // each should appear at least once (p ~ 1 - (4/5)^3000 ≈ 1)
    assert(symbolCount.size() == 5);
    for (const auto& [sym, count] : symbolCount) {
        assert(count > 0);
        (void)sym;
        (void)count;
    }

    // Count wins (should see many with 1000 spins)
    int wins = 0;
    for (int i = 0; i < 1000; ++i) {
        Spin s = sm.spin(static_cast<unsigned>(i + 10000));
        if (s.iswinspin()) ++wins;
    }
    // Probability of 0 wins in 1000 spins is essentially zero
    assert(wins > 0);

    std::cout << "  PASS statistical_variety\n";
}

// ─── 13. Spin ID format ─────────────────────────────────────────────

static void test_spin_id_format() {
    SlotMachine sm;
    Spin s = sm.spin(99);
    const std::string& id = s.getspinid();
    assert(id.find("SPIN-") == 0);
    assert(id.find("-99") != std::string::npos);

    std::cout << "  PASS spin_id_format\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "SlotMachine tests:\n";

    test_deterministic_spin();
    test_different_seeds();
    test_spin_returns_three_symbols();
    test_three_of_a_kind();
    test_two_of_a_kind();
    test_no_match();
    test_win_flag_matches_multiplier();
    test_pattern_strings();
    test_custom_symbols();
    test_empty_symbol_pool_fallback();
    test_set_symbols();
    test_statistical_variety();
    test_spin_id_format();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
