#pragma once

#include<iostream>
#include<vector>
#include<string>
using namespace std;

class GameRecord {

private:
    int gameid;
    string playerid;
    string betid;
    string spinid;
    string timestamp;
    double betamt;
    string bettype;
    vector<string> spinsymbols;
    bool win;
    double payout;
    double balbefore;
    double balafter;
    double fraud;

public:

    GameRecord(
        int gid,
        const string& pid,
        const string& bid,
        const string& sid,
        const string& time,
        double amt,
        const string& btype,
        const vector<string>& symbols,
        bool w,
        double pay,
        double before,
        double after,
        double fraud
    );

    // Getters
    int getgameid() const;
    string getplayerid() const;
    string getbetid() const;
    string getspinid() const;
    string gettime() const;
    double getbetamt() const;
    string getbettype() const;
    vector<string> getspinsymbols() const;
    bool iswin() const;
    double getpayout() const;
    double getbalbefore() const;
    double getbalafter() const;
    double getfraud() const;

    // Setters
    void setgameid(int g);
    void setplayerid(const string& p);
    void setbetid(const string& bi);
    void setspinid(const string& s);
    void settimestamp(const string& t);
    void setbetamt(double amt);
    void setbettype(const string& btype);
    void setspinsymbols(const vector<string>& symb);
    void setwin(bool w);
    void setpayout(double pay);
    void setbalbefore(double b);
    void setbalafter(double a);
    void setfraudscore(double f);
};