#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class Bet{
    protected:
    string betid;
    string pid;
    double betamt;
    std::chrono::year_month_day time;
    std::chrono::time_point<std::chrono::system_clock> now=std::chrono::system_clock::now();
    int bettype;
    int risk;
    double expected;
    double recom;

    public:
    Bet(string bid,string p,double a ,std::chrono::year_month_day t,int btype,int r,double e,double re){
        betid=bid;
        pid=p;
        betamt=a;
        time=t;
        bettype=btype;
        risk=r;
        expected =e;
        recom= re;

    }


};



int main(){
   
    /*Bet ID
Player ID
Timestamp
Bet amount
Bet type
Exact Match
Symbol Match
Pattern Match
Risk level
Expected value
Recommended amount (from strategy engine)*/
    return 0;
}