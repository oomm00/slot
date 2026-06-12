#include "GameRecord.h"

GameRecord::GameRecord(int gid, const std::string& pid, const std::string& bid,
                       const std::string& sid, const std::string& t,
                       double amt, BetType btype,
                       const std::vector<std::string>& pred,
                       const std::vector<std::string>& sym,
                       bool w, double pay, double b, double a, double f)
    : gameid(gid), playerid(pid), betid(bid), spinid(sid),
      timestamp(t), betamt(amt), bettype(btype), prediction(pred),
      spinsymbols(sym), win(w), payout(pay), balbefore(b), balafter(a),
      fraud(f) {}

int GameRecord::getgameid() const { return gameid; }
std::string GameRecord::getplayerid() const { return playerid; }
std::string GameRecord::getbetid() const { return betid; }
std::string GameRecord::getspinid() const { return spinid; }
std::string GameRecord::gettime() const { return timestamp; }
double GameRecord::getbetamt() const { return betamt; }
BetType GameRecord::getbettype() const { return bettype; }
std::vector<std::string> GameRecord::getprediction() const { return prediction; }
std::vector<std::string> GameRecord::getspinsymbols() const { return spinsymbols; }
bool GameRecord::iswin() const { return win; }
double GameRecord::getpayout() const { return payout; }
double GameRecord::getbalbefore() const { return balbefore; }
double GameRecord::getbalafter() const { return balafter; }
double GameRecord::getfraud() const { return fraud; }

void GameRecord::setgameid(int g) { gameid = g; }
void GameRecord::setplayerid(const std::string& p) { playerid = p; }
void GameRecord::setbetid(const std::string& bi) { betid = bi; }
void GameRecord::setspinid(const std::string& s) { spinid = s; }
void GameRecord::settimestamp(const std::string& t) { timestamp = t; }
void GameRecord::setbetamt(double amt) { if (amt >= 0) betamt = amt; }
void GameRecord::setbettype(BetType btype) { bettype = btype; }
void GameRecord::setprediction(const std::vector<std::string>& pred) { prediction = pred; }
void GameRecord::setspinsymbols(const std::vector<std::string>& symb) { spinsymbols = symb; }
void GameRecord::setwin(bool w) { win = w; }
void GameRecord::setpayout(double pay) { payout = pay; }
void GameRecord::setbalbefore(double b) { balbefore = b; }
void GameRecord::setbalafter(double a) { balafter = a; }
void GameRecord::setfraudscore(double f) { if (f >= 0) fraud = f; }
