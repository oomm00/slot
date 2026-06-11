#include<iostream>
#include <chrono>
#include "strategyreport.h"
using namespace std;

strategyreport::strategyreport(
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
): playerid(pid),time(std::chrono::system_clock::now()),currbal(bal),recomstrategy(rs),recomamt(ra),expectreturn(er),risk(r)
,dp(d),greedy(g),kelly(k),confidence(conf){}

// Getters

string strategyreport::getplayerid() const {
    return playerid;
}

std::chrono::time_point<std::chrono::system_clock>
strategyreport::gettime() const {
    return time;
}

double strategyreport::getcurrbal() const {
    return currbal;
}

string strategyreport::getrecomstrategy() const {
    return recomstrategy;
}

double strategyreport::getrecomamt() const {
    return recomamt;
}

double strategyreport::getexpectreturn() const {
    return expectreturn;
}

double strategyreport::getrisk() const {
    return risk;
}

string strategyreport::getdp() const {
    return dp;
}

string strategyreport::getgreedy() const {
    return greedy;
}

string strategyreport::getkelly() const {
    return kelly;
}

double strategyreport::getconfidence() const {
    return confidence;
}

// Setters

void strategyreport::setplayerid(const string& pid) {
    playerid = pid;
}

void strategyreport::setcurrbal(double bal) {
    currbal = bal;
}

void strategyreport::setrecomstrategy(const string& rs) {
    recomstrategy = rs;
}

void strategyreport::setrecomamt(double ra) {
    recomamt = ra;
}

void strategyreport::setexpectreturn(double er) {
    expectreturn = er;
}

void strategyreport::setrisk(double r) {
    risk = r;
}

void strategyreport::setdp(const string& d) {
    dp = d;
}

void strategyreport::setgreedy(const string& g) {
    greedy = g;
}

void strategyreport::setkelly(const string& k) {
    kelly = k;
}

void strategyreport::setconfidence(double conf) {
    if(conf >= 0 && conf <= 100)
        confidence = conf;
}