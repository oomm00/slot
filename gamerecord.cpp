#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class GameRecord{
    private:
  
    int gameID;
    int playerID;
    int betID;
    int spinID;
    string timestamp;
    double betAmount;
    string betType;
    vector<string> spinSymbols;
    bool iswin;
    double payout;
    double balanceBefore;
    double balanceAfter;
    double fraudScore;

    public:
    GameRecord(int g,int p,int b,int s,string t,double a,string bt,vector<string> ss,bool w,double pa,double bb,double ba,double fs){
        gameID=g;
        playerID=p;
        betID=b;
        spinID=s;
        timestamp=t;
        betAmount=a;
        betType=bt;
        spinSymbols=ss;
        iswin=w;
        payout=pa;
        balanceBefore=bb;
        balanceAfter=ba;
        fraudScore=fs;

    }
};
int main(){
      /*


Game ID
Player ID
Bet ID
Spin ID
Timestamp
Bet amount
Bet type
Spin symbols
Win/Loss
Payout
Balance before
Balance after
Fraud score at that moment*/
    return 0;
}