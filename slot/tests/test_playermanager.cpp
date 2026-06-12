#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "PlayerManager.h"

static std::string tempDir() {
    const char* tmp = std::getenv("TEMP");
    if (!tmp) tmp = ".";
    auto now = std::chrono::system_clock::to_time_t(
        std::chrono::system_clock::now());
    std::string dir = std::string(tmp) + "/playermanager_test/" + std::to_string(now);
    std::filesystem::create_directories(dir);
    return dir;
}

static void cleanup(const std::string& dir) {
    std::filesystem::remove_all(dir);
}

// ─── 1. Add player ──────────────────────────────────────────────────

static void test_add_player() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    assert(mgr.addPlayer(player("p1", "Alice", 1000.0)));
    assert(mgr.addPlayer(player("p2", "Bob", 500.0)));
    assert(mgr.addPlayer(player("p3", "Charlie", 250.0)));

    auto all = mgr.getAllPlayers();
    assert(all.size() == 3);

    cleanup(dir);
    std::cout << "  PASS add_player\n";
}

// ─── 2. Duplicate prevention ────────────────────────────────────────

static void test_duplicate_prevention() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    assert(mgr.addPlayer(player("p1", "Alice", 100.0)));
    assert(!mgr.addPlayer(player("p1", "Alice2", 200.0)));  // same ID
    assert(mgr.getAllPlayers().size() == 1);

    cleanup(dir);
    std::cout << "  PASS duplicate_prevention\n";
}

// ─── 3. Remove player ───────────────────────────────────────────────

static void test_remove_player() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 100.0));
    mgr.addPlayer(player("p2", "Bob", 200.0));

    assert(mgr.removePlayer("p1"));
    assert(mgr.getAllPlayers().size() == 1);
    assert(mgr.getPlayer("p1") == nullptr);

    // Removing non-existent returns false
    assert(!mgr.removePlayer("nonexistent"));

    cleanup(dir);
    std::cout << "  PASS remove_player\n";
}

// ─── 4. Get player ──────────────────────────────────────────────────

static void test_get_player() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 100.0));

    auto* p = mgr.getPlayer("p1");
    assert(p != nullptr);
    assert(p->getplayerid() == "p1");
    assert(p->getname() == "Alice");
    assert(std::abs(p->getbal() - 100.0) < 0.001);

    assert(mgr.getPlayer("nonexistent") == nullptr);

    cleanup(dir);
    std::cout << "  PASS get_player\n";
}

// ─── 5. Deposit ─────────────────────────────────────────────────────

static void test_deposit() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 100.0));

    assert(mgr.deposit("p1", 50.0));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 150.0) < 0.001);

    assert(mgr.deposit("p1", 0.01));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 150.01) < 0.001);

    // Zero and negative deposits rejected
    assert(!mgr.deposit("p1", 0.0));
    assert(!mgr.deposit("p1", -10.0));

    // Non-existent player
    assert(!mgr.deposit("nonexistent", 50.0));

    cleanup(dir);
    std::cout << "  PASS deposit\n";
}

// ─── 6. Withdraw ────────────────────────────────────────────────────

static void test_withdraw() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 200.0));

    assert(mgr.withdraw("p1", 50.0));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 150.0) < 0.001);

    // Overdraft rejected
    assert(!mgr.withdraw("p1", 200.0));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 150.0) < 0.001);

    // Zero and negative rejected
    assert(!mgr.withdraw("p1", 0.0));
    assert(!mgr.withdraw("p1", -10.0));

    // Non-existent player
    assert(!mgr.withdraw("nonexistent", 10.0));

    cleanup(dir);
    std::cout << "  PASS withdraw\n";
}

// ─── 7. Update balance ──────────────────────────────────────────────

static void test_update_balance() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 100.0));

    assert(mgr.updateBalance("p1", 500.0));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 500.0) < 0.001);

    // Zero is valid
    assert(mgr.updateBalance("p1", 0.0));
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 0.0) < 0.001);

    // Negative rejected
    assert(!mgr.updateBalance("p1", -1.0));

    // Non-existent player
    assert(!mgr.updateBalance("nonexistent", 100.0));

    cleanup(dir);
    std::cout << "  PASS update_balance\n";
}

// ─── 8. GetAllPlayers ───────────────────────────────────────────────

static void test_get_all_players() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    assert(mgr.getAllPlayers().empty());

    mgr.addPlayer(player("p1", "A", 10.0));
    mgr.addPlayer(player("p2", "B", 20.0));
    mgr.addPlayer(player("p3", "C", 30.0));

    auto all = mgr.getAllPlayers();
    assert(all.size() == 3);

    // Verify IDs are present
    bool foundP1 = false, foundP2 = false, foundP3 = false;
    for (const auto& p : all) {
        if (p.getplayerid() == "p1") foundP1 = true;
        if (p.getplayerid() == "p2") foundP2 = true;
        if (p.getplayerid() == "p3") foundP3 = true;
    }
    assert(foundP1 && foundP2 && foundP3);

    cleanup(dir);
    std::cout << "  PASS get_all_players\n";
}

// ─── 9. Save and load players via CSV ───────────────────────────────

static void test_csv_persistence() {
    auto dir = tempDir();
    {
        PlayerManager mgr{CSVStorage(dir)};
        mgr.addPlayer(player("p1", "Alice", 1000.0));
        mgr.addPlayer(player("p2", "Bob", 500.0));

        mgr.deposit("p1", 200.0);  // balance: 1200
        mgr.withdraw("p2", 50.0);  // balance: 450

        assert(mgr.savePlayers());
    }

    // New manager loads from the same directory
    {
        PlayerManager mgr{CSVStorage(dir)};
        assert(mgr.loadPlayers());

        auto all = mgr.getAllPlayers();
        assert(all.size() == 2);

        auto* p1 = mgr.getPlayer("p1");
        assert(p1 != nullptr);
        assert(p1->getname() == "Alice");
        assert(std::abs(p1->getbal() - 1200.0) < 0.001);

        auto* p2 = mgr.getPlayer("p2");
        assert(p2 != nullptr);
        assert(p2->getname() == "Bob");
        assert(std::abs(p2->getbal() - 450.0) < 0.001);
    }

    cleanup(dir);
    std::cout << "  PASS csv_persistence\n";
}

// ─── 10. Load from non-existent directory returns empty ──────────────

static void test_load_empty_when_no_file() {
    auto dir = tempDir() + "/nonexistent";
    PlayerManager mgr{CSVStorage(dir)};

    assert(mgr.loadPlayers());        // succeeds with empty state
    assert(mgr.getAllPlayers().empty());

    std::filesystem::remove_all(dir);
    std::cout << "  PASS load_empty_when_no_file\n";
}

// ─── 11. Mixed operations ───────────────────────────────────────────

static void test_mixed_operations() {
    auto dir = tempDir();
    PlayerManager mgr{CSVStorage(dir)};

    mgr.addPlayer(player("p1", "Alice", 500.0));
    mgr.deposit("p1", 100.0);                        // 600
    mgr.withdraw("p1", 250.0);                       // 350
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 350.0) < 0.001);

    mgr.updateBalance("p1", 1000.0);                 // 1000
    assert(std::abs(mgr.getPlayer("p1")->getbal() - 1000.0) < 0.001);

    mgr.removePlayer("p1");
    assert(mgr.getPlayer("p1") == nullptr);
    assert(mgr.getAllPlayers().empty());

    cleanup(dir);
    std::cout << "  PASS mixed_operations\n";
}

// ─── Main ────────────────────────────────────────────────────────────

int main() {
    std::cout << "PlayerManager tests:\n";

    test_add_player();
    test_duplicate_prevention();
    test_remove_player();
    test_get_player();
    test_deposit();
    test_withdraw();
    test_update_balance();
    test_get_all_players();
    test_csv_persistence();
    test_load_empty_when_no_file();
    test_mixed_operations();

    std::cout << "\nAll tests passed.\n";
    return 0;
}
