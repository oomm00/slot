#include<iostream>
#include<vector>
#include<string>
#include<chrono>
#include "gamerecord.h"

GameRecord::GameRecord(
    int gid,
    const string& pid,
    const string& bid,
    const string& sid,
    const string& t,
    double amt,
    const string& btype,
    const vector<string>& sym,
    bool w,
    double pay,
    double b,
    double a,
    double f
): gameid(gid),playerid(pid),betid(bid),spinid(sid),timestamp(t),betamt(amt),bettype(btype),spinsymbols(sym),win(w),payout(pay),
balbefore(b),balafter(a),fraud(f)
{
}

// Getters
int GameRecord::getgameid() const{return gameid;}
string GameRecord::getplayerid() const{ return playerid;}
string GameRecord::getbetid() const { return betid;}
string GameRecord::getspinid() const {return spinid;}
string GameRecord::gettime() const {return timestamp;}
double GameRecord::getbetamt() const {return betamt;}
string GameRecord::getbettype() const {return bettype;}
vector<string> GameRecord::getspinsymbols() const {return spinsymbols;}
bool GameRecord::iswin() const {return win;}
double GameRecord::getpayout() const { return payout;}
double GameRecord::getbalbefore() const { return balbefore;}
double GameRecord::getbalafter() const {return balafter;}
double GameRecord::getfraud() const { return fraud;}

// Setters
void GameRecord::setgameid(int gid) { gameid = gid;}
void GameRecord::setplayerid(const string& pid) { playerid = pid;}
void GameRecord::setbetid(const string& bid) {betid = bid;}
void GameRecord::setspinid(const string& sid) { spinid = sid;}
void GameRecord::settimestamp(const string& time) {timestamp = time;}
void GameRecord::setbetamt(double amt) {if(amt >= 0)betamt = amt;}
void GameRecord::setbettype(const string& btype) { bettype = btype;}
void GameRecord::setspinsymbols(const vector<string>& symbols) { spinsymbols = symbols;}
void GameRecord::setwin(bool w) {win = w;}
void GameRecord::setpayout(double pay) { payout = pay;}
void GameRecord::setbalbefore(double b) { balbefore = b;}
void GameRecord::setbalafter(double a) {balafter = a;}
void GameRecord::setfraudscore(double f) { if(f>=0)fraud=f;}