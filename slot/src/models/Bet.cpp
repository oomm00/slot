#include<iostream>
#include<vector>
#include <chrono>
#include "Bet.h"
using namespace std;

Bet::Bet(const string& bid,const string& p,double a ,std::chrono::year_month_day t,int btype,int r,double e,double re):betid(bid),pid(p),betamt(a),time(t),bettype(btype),risk(r),expected(e),recom(re){}
  
//Getters
string Bet::getbetid() const { return betid; }
string Bet::getpid() const { return pid; }
double Bet::getbetamt() const { return betamt; }
std::chrono::year_month_day Bet::gettime() const { return time; }
int Bet::getbtype() const { return bettype; }
int Bet::getrisk() const { return risk; }
double Bet::getexpvalue() const { return expected; }
double Bet::getrecamt() const { return recom; }
    
   //Seters 

void Bet::setbetid(const std::string& bid) { betid = bid; }
void Bet::setplayerid(const std::string& pid) { this->pid = pid; }
void Bet::settime(std::chrono::year_month_day t) { time = t; }
void Bet::setbtype(int btype) { bettype = btype; }
void Bet::setexpvalue(double e) { expected = e; }
void Bet::setrecamt(double re) { recom = re; }


void Bet::setbetamt(double amt)
{
    if (amt >= 0)
        betamt = amt;
}
void Bet::setrisk(int r)
{
    if (r >= 0 && r <= 100)
        risk = r;
}
