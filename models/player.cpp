#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class Player{
    private:
    string name;
    string id;
    string username;
    std::chrono::year_month_day time;
    double balance;
    int totalegamesplayed=0;
    double wins=0;
    double loss=0;
    int wincount=0;
    int losscount=0;
    int risk=1;
    int rank;
    public:
    Player(string n,string i,string u,std::chrono::year_month_day t,double bal,int r):name(n),id(i),username(u),time(t),balance(bal),rank(r){}
    string getname(){
        return name;
    }
    string getid(){
        return id;
    }
    string getusername(){
        return username;
    }
    std::chrono::year_month_day gettime(){
        return time;
    }
    double getbal(){
        return balance;
    }
    int getgamesplayed(){
        return totalegamesplayed;
    }
    double getwins(){
        return wins;
    }
    double getloss(){
        return loss;
    }
    int getwcount(){
        return wincount;
    }
    int getlcount(){
        return losscount;
    }
    int getrisk(){
        return risk;
    }
    int getrank(){
        return rank;
    }
    void gameplayed(){
        totalegamesplayed++;
    }
    void gamew(double amt){
        wins+=amt;
        wincount++;
    }
    void gamel(double amt){
        losscount++;
        loss+=amt;
    }
    void rankup(int r){
        rank=r;
    }
    void risk(int r){
        risk =r;
    }

    
    

};


int main(){
   
/*Account creation date
Current balance
Total games played
Total amount wagered
Total winnings
Total losses
Win count
Loss count
Biggest win
Biggest loss
Suspicious/Fraud flag
Risk score
Current rank*/
    return 0;
}
