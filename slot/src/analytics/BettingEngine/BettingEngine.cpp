#include "BettingEngine.h"

#include <chrono>
#include <cmath>
#include <ctime>
#include <functional>
#include <iomanip>
#include <sstream>
#include <stdexcept>

#include "GameLogger.h"
#include "GameRecord.h"
#include "PlayerManager.h"
#include "SlotMachine.h"

BettingEngine::BettingEngine(PlayerManager& pm, SlotMachine& sm, GameLogger& logger)
    : playerManager_(pm), slotMachine_(sm), gameLogger_(logger),
      nextBetId_(0), nextGameId_(0), totalBets_(0), totalWagered_(0.0),
      totalPayouts_(0.0), totalWins_(0), totalLosses_(0) {}

bool BettingEngine::validateBet(const player& p, double amount) {
    return amount > 0.0 && p.getbal() >= amount;
}

Bet BettingEngine::placeBet(const std::string& playerId, double amount,
                             BetType betType, const std::vector<std::string>& prediction) {
    player* p = playerManager_.getPlayer(playerId);
    if (!p)
        throw std::invalid_argument("BettingEngine::placeBet – player not found: " + playerId);
    if (!validateBet(*p, amount))
        throw std::invalid_argument("BettingEngine::placeBet – invalid amount " + std::to_string(amount));
    if (!playerManager_.withdraw(playerId, amount))
        throw std::runtime_error("BettingEngine::placeBet – withdrawal failed");

    std::string bid = "BET-" + std::to_string(nextBetId_++);
    double ev = amount * PayoutConfig::getMultiplier(betType);
    Bet bet(bid, playerId, amount, currentDate(), betType, prediction, 50, ev, amount);

    size_t idx = betHistory_.size();
    betHistory_.push_back(bet);
    playerBetIndex_[playerId].push_back(idx);
    ++totalBets_;
    totalWagered_ += amount;
    return bet;
}

double BettingEngine::calculatePayout(double betAmount, double multiplier) {
    return betAmount * multiplier;
}

bool BettingEngine::processSpin(const std::string& playerId, double amount,
                                 BetType betType, const std::vector<std::string>& prediction,
                                 SpinCallback callback) {
    player* p = playerManager_.getPlayer(playerId);
    if (!p) return false;
    if (!validateBet(*p, amount)) return false;

    double before = p->getbal();
    if (!playerManager_.withdraw(playerId, amount)) return false;

    Spin spinResult = slotMachine_.spin();
    std::vector<std::string> result = spinResult.getsymbols();

    double multiplier = SlotMachine::evaluateBet(betType, result, prediction);
    double payout = calculatePayout(amount, multiplier);
    bool win = multiplier > 0.0;

    if (win) playerManager_.deposit(playerId, payout);
    double after = playerManager_.getPlayer(playerId)->getbal();

    // Update player stats
    p->settotalwagered(p->gettotalwagered() + amount);
    p->setgamesplayed(p->getgamesplayed() + 1);
    if (win) {
        p->setcwins(p->getcwins() + 1);
        p->settwon(p->gettwon() + payout);
        if (payout > p->getbwin()) p->setbwin(payout);
    } else {
        p->setcloss(p->getcloss() + 1);
        p->settlost(p->gettlost() + amount);
        if (amount > p->getbloss()) p->setbloss(amount);
    }

    std::string betId = "BET-" + std::to_string(nextBetId_++);
    Bet bet(betId, playerId, amount, currentDate(), betType, prediction, 50,
            amount * PayoutConfig::getMultiplier(betType), amount);
    size_t idx = betHistory_.size();
    betHistory_.push_back(bet);
    playerBetIndex_[playerId].push_back(idx);

    double fraudScore = 0.0;
    if (win && multiplier > 20.0) fraudScore = std::min(100.0, fraudScore + 20.0);

    GameRecord rec(nextGameId_++, playerId, betId, spinResult.getspinid(),
                   currentTimestamp(), amount, betType, prediction,
                   result, win, payout, before, after, fraudScore);
    gameLogger_.logGame(rec);

    ++totalBets_;
    totalWagered_ += amount;
    totalPayouts_ += payout;
    if (win) ++totalWins_;
    else ++totalLosses_;

    if (callback) {
        SpinResult sr;
        sr.success = true;
        sr.playerId = playerId;
        sr.spinId = spinResult.getspinid();
        sr.symbols = result;
        sr.prediction = prediction;
        sr.pattern = spinResult.getwinpattern();
        sr.betType = betType;
        sr.multiplier = multiplier;
        sr.betAmount = amount;
        sr.payout = payout;
        sr.balanceBefore = before;
        sr.balanceAfter = after;
        sr.isWin = win;
        sr.fraudScore = fraudScore;
        callback(sr);
    }

    return true;
}

BettingEngine::Stats BettingEngine::getStats() const {
    Stats s;
    s.totalBets = totalBets_;
    s.totalWagered = totalWagered_;
    s.totalPayouts = totalPayouts_;
    s.houseProfit = totalWagered_ - totalPayouts_;
    long long decided = totalWins_ + totalLosses_;
    s.winRate = (decided > 0)
                    ? (static_cast<double>(totalWins_) / static_cast<double>(decided)) * 100.0
                    : 0.0;
    return s;
}

std::vector<Bet> BettingEngine::getPlayerBets(const std::string& playerId) const {
    std::vector<Bet> result;
    auto it = playerBetIndex_.find(playerId);
    if (it == playerBetIndex_.end()) return result;
    result.reserve(it->second.size());
    for (size_t i : it->second) result.push_back(betHistory_[i]);
    return result;
}

std::string BettingEngine::currentTimestamp() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::chrono::year_month_day BettingEngine::currentDate() {
    auto now = std::chrono::system_clock::now();
    auto dp = std::chrono::floor<std::chrono::days>(now);
    return std::chrono::year_month_day{dp};
}
