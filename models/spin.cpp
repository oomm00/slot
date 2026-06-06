#include<iostream>
#include<vector>
using namespace std;
#include "spin.h"

Spin::Spin(
    const string& s,
    const vector<string>& symb,
    const string& winp,
    double mup,
    unsigned int seed,
    bool winspi
):spinid(s),symbols(symb),winpattern(winp),multiplier(mup),seed(seed),winspin(winspi){}

// Getters
string Spin::getspinid()const{return spinid;}


vector<string> Spin::getsymbols() const {
    return symbols;
}

string Spin::getwinpattern() const {
    return winpattern;
}

double Spin::getmultiplier() const {
    return multiplier;
}

unsigned int Spin::getseed() const {
    return seed;
}

bool Spin::iswinspin() const {
    return winspin;
}

// Setters

void Spin::setspinid(const std::string& s){spinid = s;}
void Spin::setsymbols(const vector<string>& sym) {symbols = sym;}

void Spin::setwinpattern(const std::string& p) {winpattern = p;}

void Spin::setmultiplier(double m) {
    if (m >= 0)multiplier = m;
}

void Spin::setseed(unsigned int s) {
    seed = s;
}

void Spin::setwinspin(bool w) {
    winspin = w;
}
