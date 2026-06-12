#include "ApplicationController.h"

#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "FraudDetector.h"
#include "GameRecord.h"
#include "PasswordHasher.h"
#include "ProbabilityAnalyzer.h"
#include "SearchEngine.h"

ApplicationController::ApplicationController() {}

ApplicationController::~ApplicationController() {
    if (initialised_) {
        try { saveSystem(); } catch (...) {}
    }
    delete se_;
    delete fd_;
    delete pa_;
    delete lb_;
    delete be_;
    delete gl_;
    delete sm_;
    delete pm_;
    delete storage_;
}

void ApplicationController::initialise(const std::string& dataDir) {
    if (initialised_) return;
    dataDir_ = dataDir;
    storage_ = new CSVStorage(dataDir);
    pm_ = new PlayerManager(*storage_);
    sm_ = new SlotMachine();
    gl_ = new GameLogger();
    be_ = new BettingEngine(*pm_, *sm_, *gl_);
    lb_ = new Leaderboard();
    pa_ = new ProbabilityAnalyzer();
    fd_ = new FraudDetector();
    se_ = new SearchEngine(*pm_, *gl_, *be_);
    initialised_ = true;
}

bool ApplicationController::loadSystem() {
    if (!initialised_) return false;
    bool ok = true;
    try { ok &= pm_->loadPlayers(); } catch (...) { ok = false; }
    try {
        auto loadedGames = storage_->loadGames();
        for (const auto& g : loadedGames) gl_->logGame(g);
    } catch (...) { ok = false; }
    if (!pm_->getAllPlayers().empty()) refreshLeaderboard();
    se_->rebuildIndices();
    return ok;
}

bool ApplicationController::saveSystem() {
    if (!initialised_) return false;
    bool ok = true;
    try { ok &= storage_->savePlayers(pm_->getAllPlayers()); } catch (...) { ok = false; }
    try {
        std::vector<Bet> allBets;
        auto players = pm_->getAllPlayers();
        for (const auto& p : players) {
            auto bets = be_->getPlayerBets(p.getplayerid());
            allBets.insert(allBets.end(), bets.begin(), bets.end());
        }
        ok &= storage_->saveBets(allBets);
    } catch (...) { ok = false; }
    try { ok &= storage_->saveGames(gl_->getAll()); } catch (...) { ok = false; }
    try {
        std::vector<strategyreport> reports;
        auto players = pm_->getAllPlayers();
        for (const auto& p : players)
            reports.push_back(pa_->reportForPlayer(p.getplayerid(), p.getbal(), 50));
        ok &= storage_->saveReports(reports);
    } catch (...) { ok = false; }
    return ok;
}

bool ApplicationController::registerPlayer(const std::string& playerId,
                                            const std::string& name,
                                            const std::string& username,
                                            double initialBalance,
                                            const std::string& passwordHash,
                                            int age,
                                            const std::string& role) {
    if (!initialised_) return false;
    player p(playerId, name, username, initialBalance, passwordHash, role, age);
    if (!pm_->addPlayer(p)) return false;
    refreshLeaderboard();
    se_->rebuildIndices();
    return true;
}

bool ApplicationController::loginPlayer(const std::string& username,
                                         const std::string& passwordHash,
                                         std::string& outPlayerId) {
    if (!initialised_) return false;
    for (const auto& p : pm_->getAllPlayers()) {
        if (p.getusername() == username && p.getPasswordHash() == passwordHash) {
            outPlayerId = p.getplayerid();
            return true;
        }
    }
    return false;
}

bool ApplicationController::depositFunds(const std::string& playerId, double amount) {
    if (!initialised_) return false;
    bool ok = pm_->deposit(playerId, amount);
    if (ok) { refreshLeaderboard(); se_->rebuildIndices(); }
    return ok;
}

bool ApplicationController::withdrawFunds(const std::string& playerId, double amount) {
    if (!initialised_) return false;
    bool ok = pm_->withdraw(playerId, amount);
    if (ok) { refreshLeaderboard(); se_->rebuildIndices(); }
    return ok;
}

double ApplicationController::getBalance(const std::string& playerId) {
    if (!initialised_) return -1.0;
    player* p = pm_->getPlayer(playerId);
    return p ? p->getbal() : -1.0;
}

bool ApplicationController::playerExists(const std::string& playerId) {
    return initialised_ && pm_->getPlayer(playerId) != nullptr;
}

bool ApplicationController::deletePlayer(const std::string& playerId) {
    if (!initialised_) return false;
    pm_->removePlayer(playerId);
    refreshLeaderboard();
    se_->rebuildIndices();
    return true;
}

std::vector<player> ApplicationController::getAllPlayers() const {
    return initialised_ ? pm_->getAllPlayers() : std::vector<player>();
}

const player* ApplicationController::getPlayerById(const std::string& playerId) const {
    return initialised_ ? pm_->getPlayer(playerId) : nullptr;
}

SpinResult ApplicationController::playSpin(const std::string& playerId, double betAmount,
                                            BetType betType,
                                            const std::vector<std::string>& prediction) {
    SpinResult result{};
    result.playerId = playerId;
    result.betAmount = betAmount;
    result.betType = betType;
    result.prediction = prediction;

    if (!initialised_) {
        result.success = false;
        result.errorMessage = "System not initialised";
        return result;
    }

    player* p = pm_->getPlayer(playerId);
    if (!p) {
        result.success = false;
        result.errorMessage = "Player not found: " + playerId;
        return result;
    }
    result.balanceBefore = p->getbal();

    if (!be_->validateBet(*p, betAmount)) {
        result.success = false;
        result.errorMessage = "Invalid bet amount or insufficient funds";
        return result;
    }

    bool spinOk = be_->processSpin(playerId, betAmount, betType, prediction,
        [&](const SpinResult& sr) { result = sr; });
    if (!spinOk) {
        result.success = false;
        result.errorMessage = "Spin processing failed";
        return result;
    }

    refreshLeaderboard();
    se_->rebuildIndices();
    result.fraudScore = runFraudCheckInternal(playerId);
    result.success = true;
    return result;
}

std::vector<player> ApplicationController::getLeaderboard(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopPlayers(n);
}

std::vector<player> ApplicationController::getTopByBalance(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopByBalance(n);
}

std::vector<player> ApplicationController::getTopByWinRate(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopByWinRate(n);
}

std::vector<player> ApplicationController::getTopByTotalWinnings(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopByTotalWinnings(n);
}

std::vector<player> ApplicationController::getTopByGamesPlayed(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopByGamesPlayed(n);
}

std::vector<player> ApplicationController::getTopByBiggestWin(size_t n) {
    if (!initialised_) return {};
    refreshLeaderboard();
    return lb_->getTopByBiggestWin(n);
}

strategyreport ApplicationController::generatePlayerReport(const std::string& playerId,
                                                            size_t recentSpins) {
    if (!initialised_)
        return strategyreport(playerId, 0.0, "error", 0.0, 0.0, 1.0, "0", "0", "0", 0.0);
    player* p = pm_->getPlayer(playerId);
    double bankroll = p ? p->getbal() : 0.0;
    return pa_->reportForPlayer(playerId, bankroll, recentSpins);
}

double ApplicationController::runFraudCheck(const std::string& playerId) {
    return initialised_ ? runFraudCheckInternal(playerId) : 0.0;
}

double ApplicationController::runFraudCheckInternal(const std::string& playerId) {
    auto bets = be_->getPlayerBets(playerId);
    auto games = gl_->getPlayerHistory(playerId);
    return fd_->analyzePlayer(*pm_, playerId, bets, games);
}

ProbabilityAnalyzer::PerformanceReport ApplicationController::getPerformanceReport() {
    return initialised_ ? pa_->performanceReport(be_->getStats())
                        : ProbabilityAnalyzer::PerformanceReport();
}

FraudDetector::FraudStats ApplicationController::getFraudStats() {
    return initialised_ ? fd_->getStats() : FraudDetector::FraudStats();
}

const player* ApplicationController::searchPlayerById(const std::string& playerId) {
    return initialised_ ? se_->searchPlayerById(playerId) : nullptr;
}

std::vector<player> ApplicationController::searchPlayerByName(const std::string& prefix) {
    return initialised_ ? se_->searchPlayerByName(prefix) : std::vector<player>();
}

GameRecord ApplicationController::searchGameById(int gameId) {
    if (!initialised_) throw std::out_of_range("System not initialised");
    return se_->searchGameById(gameId);
}

Bet ApplicationController::searchBetById(const std::string& betId) {
    if (!initialised_) throw std::out_of_range("System not initialised");
    return se_->searchBetById(betId);
}

std::vector<GameRecord> ApplicationController::searchGamesByDateRange(const std::string& start,
                                                                       const std::string& end) {
    return initialised_ ? se_->searchGamesByDateRange(start, end) : std::vector<GameRecord>();
}

std::vector<GameRecord> ApplicationController::searchSymbolPattern(
    const std::vector<std::string>& pattern) {
    return initialised_ ? se_->searchSymbolPattern(pattern) : std::vector<GameRecord>();
}

std::vector<std::vector<Bet>> ApplicationController::searchBetPattern(
    const std::vector<double>& pattern) {
    return initialised_ ? se_->searchBetPattern(pattern) : std::vector<std::vector<Bet>>();
}

void ApplicationController::refreshLeaderboard() {
    auto players = pm_->getAllPlayers();
    lb_->update(players);
}
