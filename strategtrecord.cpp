#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class StrategyReport{

    private:
    string playerID;
    std::chrono::time_point<std::chrono::system_clock> now;
    double currentBalance;
    string recomstrategy;
    double recomamt;
    double expectreturn;
    double risk;
    string dp;
    string greedy;
    string kellyCriterion;
    double confidence;
    public:
    StrategyReport(string p, string a, double bal, string recoms, double recomamt, double e, double r, string dp, string greedy, string k, double c){
        playerID = p;
        now = std::chrono::system_clock::now();
        currentBalance = bal;
        recomstrategy = recoms;
        recomamt = recomamt;
        expectreturn = e;
        risk = r;
        dp = dp;
        greedy = greedy;
        kellyCriterion = k;
        confidence = c;
    }

  
};
int main(){
      /*
Player ID
Analysis timestamp
Current balance
Recommended strategy
Recommended bet amount
Expected return
Risk score
DP result
Greedy result
Kelly Criterion result
Confidence level
    return 0;
}