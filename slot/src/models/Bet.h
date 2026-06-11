#pragma once
#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class Bet{
    private:
    string betid;
    string pid;
    double betamt;
    std::chrono::year_month_day time;
    int bettype;
    int risk;
    double expected;
    double recom;

    public:
    Bet(const string& bid,const string& p,double a ,std::chrono::year_month_day t,int btype,int r,double e,double re);

    //Getters
    string getbetid() const;
    string getpid() const;
    double getbetamt() const;
    std::chrono::year_month_day gettime() const;
    int getbtype() const;
    int getrisk() const;
    double getexpvalue() const;
    double getrecamt() const;
    //Setters
    void setbetid(const string& bid);   
    void setplayerid(const string& pid);
    void setbetamt(double amt);
    void settime(std::chrono::year_month_day t);
    void setbtype(int btype);
    void setrisk(int r);
    void setexpvalue(double e);
    void setrecamt(double re);

};
