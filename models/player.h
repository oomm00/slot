#pragma once
#include <string>
#include <chrono>
using namespace std;
class player {
private:
    string pid;
    string username;
    double balance;
    int gamesplayed;
    double totalwagered;
    double twon;
    double tlost;
    int cwins;
    int closs;
    double bwin;
    double bloss;
    bool fraud;
    double risk;
    int rank;
    chrono::system_clock::time_point creation;

public:
    player(
        const string& playerId,
        const string& username,
        double initialBalance
    );

    // Getters
    string getplayerid() const;
    string getname() const;
    double getbal() const;
    int getgamesplayed() const;
    double gettotalwagered() const;
    double gettwon() const;
    double gettlost() const;
    int getcwins() const;
    int getcloss() const;
    double getbwin() const;
    double getbloss() const;
    bool isfraud() const;
    double getrisk() const;
    int getrank() const;

    chrono::system_clock::time_point getcreation() const;

    // Setters
    void setname(const std::string& username);
    void setbal(double bal);
    void setfruad(bool f);
    void setrisk(double s);
    void setrank(int r);

    // Statistics Updates
    void addwager(double amount);
    void recordwin(double amount);
    void recordloss(double amount);
    double getwinrate() const;
    double getROI() const;
};