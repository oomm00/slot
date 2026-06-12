#pragma once
#include <string>
#include <vector>
#include "BetType.h"

class GameRecord {
private:
    int gameid;
    std::string playerid;
    std::string betid;
    std::string spinid;
    std::string timestamp;
    double betamt;
    BetType bettype;
    std::vector<std::string> prediction;
    std::vector<std::string> spinsymbols;
    bool win;
    double payout;
    double balbefore;
    double balafter;
    double fraud;

public:
    GameRecord(int gid, const std::string& pid, const std::string& bid,
               const std::string& sid, const std::string& time,
               double amt, BetType btype,
               const std::vector<std::string>& pred,
               const std::vector<std::string>& symbols,
               bool w, double pay, double before, double after, double fraud);

    int getgameid() const;
    std::string getplayerid() const;
    std::string getbetid() const;
    std::string getspinid() const;
    std::string gettime() const;
    double getbetamt() const;
    BetType getbettype() const;
    std::vector<std::string> getprediction() const;
    std::vector<std::string> getspinsymbols() const;
    bool iswin() const;
    double getpayout() const;
    double getbalbefore() const;
    double getbalafter() const;
    double getfraud() const;

    void setgameid(int g);
    void setplayerid(const std::string& p);
    void setbetid(const std::string& bi);
    void setspinid(const std::string& s);
    void settimestamp(const std::string& t);
    void setbetamt(double amt);
    void setbettype(BetType btype);
    void setprediction(const std::vector<std::string>& pred);
    void setspinsymbols(const std::vector<std::string>& symb);
    void setwin(bool w);
    void setpayout(double pay);
    void setbalbefore(double b);
    void setbalafter(double a);
    void setfraudscore(double f);
};
