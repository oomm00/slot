#include "Bet.h"

Bet::Bet(const std::string& bid, const std::string& p, double a,
         std::chrono::year_month_day t, BetType btype,
         const std::vector<std::string>& pred,
         int r, double e, double re)
    : betid(bid), pid(p), betamt(a), time(t), bettype(btype),
      prediction(pred), risk(r), expected(e), recom(re) {}

std::string Bet::getbetid() const { return betid; }
std::string Bet::getpid() const { return pid; }
double Bet::getbetamt() const { return betamt; }
std::chrono::year_month_day Bet::gettime() const { return time; }
BetType Bet::getbtype() const { return bettype; }
std::vector<std::string> Bet::getprediction() const { return prediction; }
int Bet::getrisk() const { return risk; }
double Bet::getexpvalue() const { return expected; }
double Bet::getrecamt() const { return recom; }

void Bet::setbetid(const std::string& bid) { betid = bid; }
void Bet::setplayerid(const std::string& pid) { this->pid = pid; }
void Bet::setbetamt(double amt) { if (amt >= 0) betamt = amt; }
void Bet::settime(std::chrono::year_month_day t) { time = t; }
void Bet::setbtype(BetType btype) { bettype = btype; }
void Bet::setprediction(const std::vector<std::string>& pred) { prediction = pred; }
void Bet::setrisk(int r) { if (r >= 0 && r <= 100) risk = r; }
void Bet::setexpvalue(double e) { expected = e; }
void Bet::setrecamt(double re) { recom = re; }
