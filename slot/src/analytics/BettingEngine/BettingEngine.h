#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>
#include <chrono>

#include "Bet.h"
#include "BetType.h"
#include "SpinResult.h"
#include "Player.h"

class GameLogger;
class PlayerManager;
class SlotMachine;

class BettingEngine {
public:
    BettingEngine(PlayerManager& pm, SlotMachine& sm, GameLogger& logger);

    bool validateBet(const player& p, double amount);
    Bet placeBet(const std::string& playerId, double amount,
                 BetType betType, const std::vector<std::string>& prediction = {});
    static double calculatePayout(double betAmount, double multiplier);

    using SpinCallback = std::function<void(const SpinResult&)>;

    bool processSpin(const std::string& playerId, double amount,
                     BetType betType = BetType::ANY_PAIR,
                     const std::vector<std::string>& prediction = {},
                     SpinCallback callback = nullptr);

    struct Stats {
        long long totalBets = 0;
        double totalWagered = 0.0;
        double totalPayouts = 0.0;
        double houseProfit = 0.0;
        double winRate = 0.0;
    };

    Stats getStats() const;
    std::vector<Bet> getPlayerBets(const std::string& playerId) const;

    const std::vector<Bet>& getAllBets() const { return betHistory_; }

private:
    PlayerManager& playerManager_;
    SlotMachine& slotMachine_;
    GameLogger& gameLogger_;

    std::vector<Bet> betHistory_;
    std::unordered_map<std::string, std::vector<size_t>> playerBetIndex_;
    int nextBetId_;
    int nextGameId_;

    long long totalBets_;
    double totalWagered_;
    double totalPayouts_;
    int totalWins_;
    int totalLosses_;

    static std::string currentTimestamp();
    static std::chrono::year_month_day currentDate();
};
