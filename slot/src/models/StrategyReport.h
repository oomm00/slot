#pragma once

#include<iostream>
#include<vector>
#include<chrono>

using namespace std;

class strategyreport {

private:
    string playerid;
    std::chrono::time_point<std::chrono::system_clock> time;
    double currbal;
    string recomstrategy;
    double recomamt;
    double expectreturn;
    double risk;
    string dp;
    string greedy;
    string kelly;
    double confidence;

public:
    strategyreport(
        const string& pid,
        double bal,
        const string& rs,
        double ra,
        double er,
        double r,
        const string& d,
        const string& g,
        const string& k,
        double conf
    );
    // Getters
    string getplayerid() const;
    std::chrono::time_point<std::chrono::system_clock> gettime() const;
    double getcurrbal() const;
    string getrecomstrategy() const;
    double getrecomamt() const;
    double getexpectreturn() const;
    double getrisk() const;
    string getdp() const;
    string getgreedy() const;
    string getkelly() const;
    double getconfidence() const;

    // Setters
    void setplayerid(const string& pid);
    void setcurrbal(double bal);
    void setrecomstrategy(const string& rs);
    void setrecomamt(double ra);
    void setexpectreturn(double er);
    void setrisk(double r);
    void setdp(const string& d);
    void setgreedy(const string& g);
    void setkelly(const string& k);
    void setconfidence(double conf);
};