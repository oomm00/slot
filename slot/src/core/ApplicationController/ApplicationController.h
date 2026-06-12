#pragma once

#include <string>
#include <vector>

#include "Bet.h"
#include "BettingEngine.h"
#include "CSVStorage.h"
#include "FraudDetector.h"
#include "GameLogger.h"
#include "GameRecord.h"
#include "Leaderboard.h"
#include "Player.h"
#include "PlayerManager.h"
#include "ProbabilityAnalyzer.h"
#include "SearchEngine.h"
#include "SlotMachine.h"
#include "SpinResult.h"
#include "StrategyReport.h"

class ApplicationController {
public:
    ApplicationController();
    ~ApplicationController();

    void initialise(const std::string& dataDir = "data");
    bool loadSystem();
    bool saveSystem();

    // ── Player Management (with auth) ──────────────────────────────
    bool registerPlayer(const std::string& playerId, const std::string& name,
                        const std::string& username, double initialBalance,
                        const std::string& passwordHash, int age,
                        const std::string& role = "player");
    bool loginPlayer(const std::string& username, const std::string& passwordHash,
                     std::string& outPlayerId);
    bool depositFunds(const std::string& playerId, double amount);
    bool withdrawFunds(const std::string& playerId, double amount);
    double getBalance(const std::string& playerId);
    bool playerExists(const std::string& playerId);
    bool deletePlayer(const std::string& playerId);
    std::vector<player> getAllPlayers() const;
    const player* getPlayerById(const std::string& playerId) const;

    // ── Game Operations (with bet types) ───────────────────────────
    SpinResult playSpin(const std::string& playerId, double betAmount,
                        BetType betType = BetType::ANY_PAIR,
                        const std::vector<std::string>& prediction = {});

    // ── Leaderboard ────────────────────────────────────────────────
    std::vector<player> getLeaderboard(size_t n);
    std::vector<player> getTopByBalance(size_t n);
    std::vector<player> getTopByWinRate(size_t n);
    std::vector<player> getTopByTotalWinnings(size_t n);
    std::vector<player> getTopByGamesPlayed(size_t n);
    std::vector<player> getTopByBiggestWin(size_t n);

    // ── Analytics ──────────────────────────────────────────────────
    strategyreport generatePlayerReport(const std::string& playerId,
                                         size_t recentSpins = 100);
    double runFraudCheck(const std::string& playerId);
    ProbabilityAnalyzer::PerformanceReport getPerformanceReport();
    FraudDetector::FraudStats getFraudStats();

    // ── Search ─────────────────────────────────────────────────────
    const player* searchPlayerById(const std::string& playerId);
    std::vector<player> searchPlayerByName(const std::string& prefix);
    GameRecord searchGameById(int gameId);
    Bet searchBetById(const std::string& betId);
    std::vector<GameRecord> searchGamesByDateRange(const std::string& start,
                                                    const std::string& end);
    std::vector<GameRecord> searchSymbolPattern(const std::vector<std::string>& pattern);
    std::vector<std::vector<Bet>> searchBetPattern(const std::vector<double>& pattern);

    // ── Module Access (for testability) ────────────────────────────
    PlayerManager& getPlayerManager() { return *pm_; }
    SlotMachine& getSlotMachine() { return *sm_; }
    GameLogger& getGameLogger() { return *gl_; }
    BettingEngine& getBettingEngine() { return *be_; }
    Leaderboard& getLeaderboardRef() { return *lb_; }
    ProbabilityAnalyzer& getAnalyzer() { return *pa_; }
    FraudDetector& getFraudDetector() { return *fd_; }
    SearchEngine& getSearchEngine() { return *se_; }

private:
    CSVStorage* storage_{};
    PlayerManager* pm_{};
    SlotMachine* sm_{};
    GameLogger* gl_{};
    BettingEngine* be_{};
    Leaderboard* lb_{};
    ProbabilityAnalyzer* pa_{};
    FraudDetector* fd_{};
    SearchEngine* se_{};

    std::string dataDir_;
    bool initialised_{};

    void refreshLeaderboard();
    double runFraudCheckInternal(const std::string& playerId);
};
