#include "Player.h"

player::player(const std::string& p, const std::string& n,
               const std::string& u, double b, const std::string& ph,
               const std::string& r, int a)
    : pid(p), name(n), username(u), passwordHash(ph), role(r), age(a),
      balance(b), gamesplayed(0), totalwagered(0), twon(0), tlost(0),
      cwins(0), closs(0), bwin(0), bloss(0),
      fraud(false), risk(0), rank(0),
      creation(std::chrono::system_clock::now()) {}

std::string player::getplayerid() const { return pid; }
std::string player::getname() const { return name; }
std::string player::getusername() const { return username; }
std::string player::getPasswordHash() const { return passwordHash; }
std::string player::getRole() const { return role; }
int player::getAge() const { return age; }
double player::getbal() const { return balance; }
int player::getgamesplayed() const { return gamesplayed; }
double player::gettotalwagered() const { return totalwagered; }
double player::gettwon() const { return twon; }
double player::gettlost() const { return tlost; }
int player::getcwins() const { return cwins; }
int player::getcloss() const { return closs; }
double player::getbwin() const { return bwin; }
double player::getbloss() const { return bloss; }
bool player::isfraud() const { return fraud; }
double player::getrisk() const { return risk; }
int player::getrank() const { return rank; }
std::chrono::system_clock::time_point player::getcreation() const { return creation; }

void player::setname(const std::string& n) { name = n; }
void player::setPasswordHash(const std::string& h) { passwordHash = h; }
void player::setRole(const std::string& r) { role = r; }
void player::setAge(int a) { age = a; }
void player::setbal(double bal) { balance = bal; }
void player::setfruad(bool f) { fraud = f; }
void player::setrisk(double s) { risk = s; }
void player::setrank(int r) { rank = r; }
void player::setgamesplayed(int n) { gamesplayed = n; }
void player::settotalwagered(double amt) { totalwagered = amt; }
void player::settwon(double amt) { twon = amt; }
void player::settlost(double amt) { tlost = amt; }
void player::setcwins(int n) { cwins = n; }
void player::setcloss(int n) { closs = n; }
void player::setbwin(double amt) { bwin = amt; }
void player::setbloss(double amt) { bloss = amt; }
void player::setcreation(std::chrono::system_clock::time_point t) { creation = t; }

void player::addwager(double amt) { totalwagered += amt; }
void player::recordwin(double amt) {
    gamesplayed++; cwins++; twon += amt;
    if (amt > bwin) bwin = amt;
    balance += amt;
}
void player::recordloss(double amt) {
    gamesplayed++; closs++; tlost += amt;
    if (amt > bloss) bloss = amt;
    balance -= amt;
}
double player::getwinrate() const {
    return gamesplayed == 0 ? 0.0 : static_cast<double>(cwins) / gamesplayed;
}
double player::getROI() const {
    return totalwagered == 0 ? 0.0 : (twon - tlost) / totalwagered;
}
