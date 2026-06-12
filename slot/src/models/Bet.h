#pragma once
#include <string>
#include <vector>
#include <chrono>
#include "BetType.h"

class Bet {
private:
    std::string betid;
    std::string pid;
    double betamt;
    std::chrono::year_month_day time;
    BetType bettype;
    std::vector<std::string> prediction;  // player's predicted symbols
    int risk;
    double expected;
    double recom;

public:
    Bet(const std::string& bid, const std::string& p, double a,
        std::chrono::year_month_day t, BetType btype,
        const std::vector<std::string>& pred,
        int r, double e, double re);

    std::string getbetid() const;
    std::string getpid() const;
    double getbetamt() const;
    std::chrono::year_month_day gettime() const;
    BetType getbtype() const;
    std::vector<std::string> getprediction() const;
    int getrisk() const;
    double getexpvalue() const;
    double getrecamt() const;

    void setbetid(const std::string& bid);
    void setplayerid(const std::string& pid);
    void setbetamt(double amt);
    void settime(std::chrono::year_month_day t);
    void setbtype(BetType btype);
    void setprediction(const std::vector<std::string>& pred);
    void setrisk(int r);
    void setexpvalue(double e);
    void setrecamt(double re);
};
