#include<iostream>
#include<vector>
#include <chrono>
using namespace std;
class spin{
    private:
    /*
Spin ID
Generated symbols
Winning pattern
Multiplier
Random seed used (optional)
Is winning spin*/
    string spinid;
    vector<string> symbols;
    string pattern;
    double multiplier;
    int seed;
    bool iswin;

    public:
    spin(string s,vector<string> sym,string p,double m,int se,bool w){
        spinid=s;
        symbols=sym;
        pattern=p;
        multiplier=m;
        seed=se;
        iswin=w;

    }
}
int main(){
    return 0;
}