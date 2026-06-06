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
    string getid() const;
    string getname() const;

    double getbal() const;

    int getgamesplayed() const;

    double getTotalWagered() const;
    double getWon() const;
    double getLost() const;

    int getwcount() const;
    int getlcount() const;

    double getBiggestWin() const;
    double getBiggestLoss() const;

    bool isfruad() const;

    double getRiskScore() const;

    int getRank() const;

    chrono::system_clock::time_point getCreationDate() const;

    // Setters
    void setname(const std::string& username);
    void setbal(double bal);
    void setfruad(bool flag);
    void setrisk(double score);
    void setRank(int rank);

    // Statistics Updates
    void addwager(double amount);

    void recordwin(double amount);

    void recordloss(double amount);

    double getwinrate() const;

    double getROI() const;
};