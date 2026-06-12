#include "SpinResult.h"
#include <algorithm>

Spin::Spin(const std::string& s, const std::vector<std::string>& sym,
           const std::string& winpat, double mult, unsigned int seed, bool winspin)
    : spinid(s), symbols(sym), winpattern(winpat),
      multiplier(mult), seed(seed), winspin(winspin) {}

std::string Spin::getspinid() const { return spinid; }
std::vector<std::string> Spin::getsymbols() const { return symbols; }
std::string Spin::getwinpattern() const { return winpattern; }
double Spin::getmultiplier() const { return multiplier; }
unsigned int Spin::getseed() const { return seed; }
bool Spin::iswinspin() const { return winspin; }

void Spin::setspinid(const std::string& s) { spinid = s; }
void Spin::setsymbols(const std::vector<std::string>& s) { symbols = s; }
void Spin::setwinpattern(const std::string& p) { winpattern = p; }
void Spin::setmultiplier(double m) { multiplier = m; }
void Spin::setseed(unsigned int s) { seed = s; }
void Spin::setwinspin(bool w) { winspin = w; }

bool Spin::evaluateExactPrediction(const std::vector<std::string>& result,
                                    const std::vector<std::string>& prediction) {
    if (result.size() != 3 || prediction.size() != 3) return false;
    return result[0] == prediction[0] &&
           result[1] == prediction[1] &&
           result[2] == prediction[2];
}

bool Spin::evaluateTripleSymbol(const std::vector<std::string>& result,
                                 const std::string& symbol) {
    if (result.size() != 3) return false;
    return result[0] == symbol && result[1] == symbol && result[2] == symbol;
}

bool Spin::evaluatePairPrediction(const std::vector<std::string>& result,
                                   const std::string& symbol) {
    if (result.size() != 3) return false;
    int count = 0;
    for (const auto& s : result) if (s == symbol) ++count;
    return count >= 2;
}

bool Spin::evaluateSymbolAppearance(const std::vector<std::string>& result,
                                     const std::string& symbol) {
    for (const auto& s : result) if (s == symbol) return true;
    return false;
}

bool Spin::evaluateAnyPair(const std::vector<std::string>& result) {
    if (result.size() != 3) return false;
    return result[0] == result[1] || result[0] == result[2] || result[1] == result[2];
}

bool Spin::evaluateAnyTriple(const std::vector<std::string>& result) {
    if (result.size() != 3) return false;
    return result[0] == result[1] && result[1] == result[2];
}
