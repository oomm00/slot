#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
#include "player.h"

player::player(
    const std::string& p,
    const std::string& u,
    double b
): pid(p),username(u),balance(b),gamesplayed(0),totalwagered(0),twon(0),tlost(0),cwins(0),closs(0),bwin(0),bloss(0),
    fraud(false),risk(0),rank(0),creation(std::chrono::system_clock::now()){}

// Getters

string player::getplayerid() const {
    return pid;
}
string player::getname() const {
    return username;
}

double player::getbal() const {
    return balance;
}

int player::getgamesplayed() const {
    return gamesplayed;
}

double player::gettotalwagered() const {
    return totalwagered;
}

double player::gettwon() const {
    return twon;
}

double player::gettlost() const {
    return tlost;
}

int player::getcwins() const {
    return cwins;
}

int player::getcloss() const {
    return closs;
}

double player::getbwin() const {
    return bwin;
}

double player::getbloss() const {
    return bloss;
}

bool player::isfraud() const {
    return fraud;
}

double player::getrisk() const {
    return risk;
}

int player::getrank() const {
    return rank;
}

std::chrono::system_clock::time_point player::getcreation() const {
    return creation;
}

// Setters
void player::setname(const std::string& u) {username = u;}
void player::setbal(double bal) {balance = bal;}
void player::setfruad(bool f) {fraud = f;}
void player::setrisk(double s) {  risk = s;}
void player::setrank(int r) {  rank = r;}

// Statistics

void player::addwager(double amt) {
    totalwagered += amt;
}

void player::recordwin(double amt) {
    gamesplayed++;
    cwins++;

    twon += amt;

    if (amt > bwin) {
        bwin = amt;
    }

    balance += amt;
}

void player::recordloss(double amt) {
    gamesplayed++;
    closs++;

    tlost += amt;

    if (amt > bloss) {
        bloss = amt;
    }

    balance -= amt;
}

double player::getwinrate() const {
    if (gamesplayed == 0) {
        return 0.0;
    }

    return static_cast<double>(cwins) / gamesplayed;
}

double player::getROI() const {
    if (totalwagered == 0) {
        return 0.0;
    }

    return (twon - tlost) / totalwagered;
}