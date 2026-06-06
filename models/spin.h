#pragma once
#include <string>
#include <vector>
using namespace std;
class Spin {
private:
    string spinid;
    vector<string> symbols;
    string winpattern;
    double multiplier;
    unsigned int seed;
    bool winspin;

public:
    Spin(const string& s,const vector<string>& sym,const string& winpat,double mult,unsigned int seed,bool winspin);

    // Getters
    string getspinid() const;
    vector<string> getsymbols() const;
    string getwinpattern() const;

    double getmultiplier() const;
    unsigned int getseed() const;

    bool iswinspin() const;

    // Setters
    void setspinid(const string& spinId);

    void setsymbols(const vector<string>& symbols);

    void setwinpattern(const string& pattern);

    void setmultiplier(double multiplier);

    void setseed(unsigned int seed);

    void setwinspin(bool winning);
};
