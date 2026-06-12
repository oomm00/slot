#pragma once
#include <string>
#include <chrono>

class player {
private:
    std::string pid;
    std::string name;
    std::string username;
    std::string passwordHash;
    std::string role;
    int age;
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
    std::chrono::system_clock::time_point creation;

public:
    player(const std::string& playerId, const std::string& name,
           const std::string& username, double initialBalance,
           const std::string& passwordHash,
           const std::string& role = "player", int age = 18);

    std::string getplayerid() const;
    std::string getname() const;
    std::string getusername() const;
    std::string getPasswordHash() const;
    std::string getRole() const;
    int getAge() const;
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
    std::chrono::system_clock::time_point getcreation() const;

    void setname(const std::string& username);
    void setPasswordHash(const std::string& hash);
    void setRole(const std::string& role);
    void setAge(int age);
    void setbal(double bal);
    void setfruad(bool f);
    void setrisk(double s);
    void setrank(int r);
    void setgamesplayed(int n);
    void settotalwagered(double amt);
    void settwon(double amt);
    void settlost(double amt);
    void setcwins(int n);
    void setcloss(int n);
    void setbwin(double amt);
    void setbloss(double amt);
    void setcreation(std::chrono::system_clock::time_point t);

    void addwager(double amount);
    void recordwin(double amount);
    void recordloss(double amount);
    double getwinrate() const;
    double getROI() const;
};
