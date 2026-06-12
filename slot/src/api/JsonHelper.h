#pragma once

#include <string>
#include <vector>

#include "Bet.h"
#include "BetType.h"
#include "GameRecord.h"
#include "Player.h"
#include "SpinResult.h"
#include "StrategyReport.h"

#include "json.hpp"

using json = nlohmann::json;

namespace JsonHelper {

inline json playerToJson(const player& p) {
    return {
        {"playerId", p.getplayerid()},
        {"name", p.getname()},
        {"username", p.getusername()},
        {"balance", p.getbal()},
        {"role", p.getRole()},
        {"age", p.getAge()},
        {"gamesPlayed", p.getgamesplayed()},
        {"totalWagered", p.gettotalwagered()},
        {"totalWon", p.gettwon()},
        {"totalLost", p.gettlost()},
        {"wins", p.getcwins()},
        {"losses", p.getcloss()},
        {"biggestWin", p.getbwin()},
        {"biggestLoss", p.getbloss()},
        {"winRate", p.getwinrate()},
        {"fraudFlagged", p.isfraud()},
        {"riskScore", p.getrisk()},
        {"rank", p.getrank()}
    };
}

inline json publicProfileToJson(const player& p) {
    return {
        {"playerId", p.getplayerid()},
        {"username", p.getusername()},
        {"gamesPlayed", p.getgamesplayed()},
        {"wins", p.getcwins()},
        {"winRate", p.getwinrate()},
        {"rank", p.getrank()}
    };
}

inline json betToJson(const Bet& b) {
    json pred = json::array();
    for (auto& s : b.getprediction()) pred.push_back(s);
    return {
        {"betId", b.getbetid()},
        {"playerId", b.getpid()},
        {"betAmount", b.getbetamt()},
        {"betType", BetTypeToString(b.getbtype())},
        {"prediction", pred},
        {"risk", b.getrisk()},
        {"expectedValue", b.getexpvalue()},
        {"recommendedAmount", b.getrecamt()}
    };
}

inline json gameRecordToJson(const GameRecord& g) {
    json syms = json::array();
    for (auto& s : g.getspinsymbols()) syms.push_back(s);
    json pred = json::array();
    for (auto& s : g.getprediction()) pred.push_back(s);
    return {
        {"gameId", g.getgameid()},
        {"playerId", g.getplayerid()},
        {"betId", g.getbetid()},
        {"spinId", g.getspinid()},
        {"timestamp", g.gettime()},
        {"betAmount", g.getbetamt()},
        {"betType", BetTypeToString(g.getbettype())},
        {"prediction", pred},
        {"symbols", syms},
        {"win", g.iswin()},
        {"payout", g.getpayout()},
        {"balanceBefore", g.getbalbefore()},
        {"balanceAfter", g.getbalafter()},
        {"fraudScore", g.getfraud()}
    };
}

inline json strategyReportToJson(const strategyreport& r) {
    return {
        {"playerId", r.getplayerid()},
        {"currentBalance", r.getcurrbal()},
        {"recommendedStrategy", r.getrecomstrategy()},
        {"recommendedAmount", r.getrecomamt()},
        {"expectedReturn", r.getexpectreturn()},
        {"risk", r.getrisk()},
        {"dpAnalysis", r.getdp()},
        {"greedyAnalysis", r.getgreedy()},
        {"kellyAnalysis", r.getkelly()},
        {"confidence", r.getconfidence()}
    };
}

inline json spinResultToJson(const SpinResult& r) {
    json syms = json::array();
    for (auto& s : r.symbols) syms.push_back(s);
    json pred = json::array();
    for (auto& s : r.prediction) pred.push_back(s);
    return {
        {"success", r.success},
        {"playerId", r.playerId},
        {"spinId", r.spinId},
        {"symbols", syms},
        {"prediction", pred},
        {"pattern", r.pattern},
        {"betType", BetTypeToString(r.betType)},
        {"multiplier", r.multiplier},
        {"betAmount", r.betAmount},
        {"payout", r.payout},
        {"balanceBefore", r.balanceBefore},
        {"balanceAfter", r.balanceAfter},
        {"win", r.isWin},
        {"fraudScore", r.fraudScore}
    };
}

} // namespace JsonHelper
