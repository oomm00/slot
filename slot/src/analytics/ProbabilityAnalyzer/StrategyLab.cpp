#include "StrategyLab.h"
#include "WeightedReel.h"
#include <algorithm>
#include <cmath>
#include <numeric>
#include <sstream>

StrategyLab::StrategyLab() {
    // Default uniform if no weights provided
}

void StrategyLab::setSymbolProbabilities(const std::unordered_map<std::string, double>& probs) {
    symProbs_ = probs;
}

double StrategyLab::getSymProb(const std::string& sym) const {
    auto it = symProbs_.find(sym);
    return (it != symProbs_.end()) ? it->second : 0.0;
}

std::vector<BetType> StrategyLab::allPredictionTypes() {
    return allBetTypes();
}

std::string StrategyLab::riskLevel(double risk) {
    if (risk < 0.1) return "Very Low";
    if (risk < 0.3) return "Low";
    if (risk < 0.5) return "Medium";
    if (risk < 0.7) return "High";
    return "Extreme";
}

// ── Per-symbol win probabilities (weighted) ────────────────────────

double StrategyLab::calcWinProbExact(const std::string& sym) const {
    double p = getSymProb(sym);
    return p * p * p;
}

double StrategyLab::calcWinProbTriple(const std::string& sym) const {
    double p = getSymProb(sym);
    return p * p * p;
}

double StrategyLab::calcWinProbPair(const std::string& sym) const {
    double p = getSymProb(sym);
    return 3.0 * p * p * (1.0 - p) + p * p * p;
}

double StrategyLab::calcWinProbAppearance(const std::string& sym) const {
    double p = getSymProb(sym);
    return 1.0 - std::pow(1.0 - p, 3);
}

double StrategyLab::calcWinProbAnyPair() const {
    double sum = 0.0;
    for (auto& [sym, p] : symProbs_) {
        sum += 3.0 * p * p * (1.0 - p) + p * p * p;
    }
    return sum;
}

double StrategyLab::calcWinProbAnyTriple() const {
    double sum = 0.0;
    for (auto& [sym, p] : symProbs_) {
        sum += p * p * p;
    }
    return sum;
}

// ── Distribution computation ───────────────────────────────────────

DistributionResult StrategyLab::computeDistribution(
    double winProb, double mult,
    double balance, double betAmount, size_t numRounds) {

    double pWin = winProb;
    double pLose = 1.0 - pWin;
    size_t M = 401;
    size_t bustIdx = M - 1;
    size_t initIdx = std::min(static_cast<size_t>(balance / 25.0), M - 2);

    std::vector<double> dp(M, 0.0);
    dp[initIdx] = 1.0;

    DistributionResult r;
    r.expectedBalances.resize(numRounds + 1);
    r.bustProbabilities.resize(numRounds + 1);
    r.stdDeviations.resize(numRounds + 1);
    r.expectedBalances[0] = balance;
    r.bustProbabilities[0] = 0.0;
    r.stdDeviations[0] = 0.0;

    for (size_t rd = 0; rd < numRounds; ++rd) {
        std::vector<double> next(M, 0.0);
        for (size_t b = 0; b < M - 1; ++b) {
            if (dp[b] < 1e-15) continue;
            double bal = static_cast<double>(b) * 25.0;

            if (bal < betAmount) {
                next[bustIdx] += dp[b];
            } else {
                size_t lossIdx = static_cast<size_t>((bal - betAmount) / 25.0);
                lossIdx = std::min(lossIdx, M - 2);
                next[lossIdx] += dp[b] * pLose;
            }

            double winBal = bal + betAmount * mult;
            size_t winIdx = static_cast<size_t>(winBal / 25.0);
            winIdx = std::min(winIdx, M - 2);
            next[winIdx] += dp[b] * pWin;
        }
        next[bustIdx] += dp[bustIdx];
        dp = next;

        double expBal = 0.0, expSq = 0.0, bustP = 0.0;
        for (size_t b = 0; b < M; ++b) {
            double bal = (b == bustIdx) ? 0.0 : static_cast<double>(b) * 25.0;
            expBal += bal * dp[b];
            expSq += bal * bal * dp[b];
            if (b == bustIdx) bustP += dp[b];
        }
        r.expectedBalances[rd + 1] = expBal;
        r.bustProbabilities[rd + 1] = bustP;
        double var = expSq - expBal * expBal;
        r.stdDeviations[rd + 1] = std::sqrt(std::max(0.0, var));
    }

    r.overallBustProb = r.bustProbabilities[numRounds];
    r.overallSurvivalProb = 1.0 - r.overallBustProb;
    r.finalExpectedBalance = r.expectedBalances[numRounds];
    return r;
}

DistributionResult StrategyLab::computeDistributionForType(
    BetType type, double balance, double betAmount,
    size_t numRounds, const std::string& symbol) {

    double wp = 0.0, mult = PayoutConfig::getMultiplier(type);

    switch (type) {
        case BetType::EXACT_PREDICTION:
        case BetType::TRIPLE_SYMBOL:
            wp = calcWinProbTriple(symbol.empty() ? "CHERRY" : symbol);
            break;
        case BetType::PAIR_PREDICTION:
            wp = calcWinProbPair(symbol.empty() ? "CHERRY" : symbol);
            break;
        case BetType::SYMBOL_APPEARANCE:
            wp = calcWinProbAppearance(symbol.empty() ? "CHERRY" : symbol);
            break;
        case BetType::ANY_PAIR:
            wp = calcWinProbAnyPair();
            break;
        case BetType::ANY_TRIPLE:
            wp = calcWinProbAnyTriple();
            break;
    }

    return computeDistribution(wp, mult, balance, betAmount, numRounds);
}

// ── Per-bet-type odds computation ─────────────────────────────────

BetTypeOdds StrategyLab::computeBetOdds(BetType type, double balance,
                                         double betAmount, size_t numRounds) {
    BetTypeOdds odds;
    odds.betType = BetTypeToString(type);
    odds.payoutMultiplier = PayoutConfig::getMultiplier(type);

    switch (type) {
        case BetType::EXACT_PREDICTION:
        case BetType::TRIPLE_SYMBOL:
        case BetType::PAIR_PREDICTION:
        case BetType::SYMBOL_APPEARANCE: {
            // Find the symbol with the best win probability for this bet type
            double bestEV = -1;
            std::string bestSym;
            for (auto& [sym, p] : symProbs_) {
                double wp = 0;
                if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL)
                    wp = calcWinProbTriple(sym);
                else if (type == BetType::PAIR_PREDICTION)
                    wp = calcWinProbPair(sym);
                else
                    wp = calcWinProbAppearance(sym);
                double ev = wp * odds.payoutMultiplier;
                if (ev > bestEV) { bestEV = ev; bestSym = sym; }
            }
            odds.winProb = bestEV / odds.payoutMultiplier;
            break;
        }
        case BetType::ANY_PAIR:
            odds.winProb = calcWinProbAnyPair();
            break;
        case BetType::ANY_TRIPLE:
            odds.winProb = calcWinProbAnyTriple();
            break;
    }

    odds.expectedValue = odds.winProb * odds.payoutMultiplier;
    odds.variance = odds.payoutMultiplier * odds.payoutMultiplier
                  * odds.winProb * (1.0 - odds.winProb);
    odds.riskScore = 1.0 - odds.winProb;
    odds.riskLevel = riskLevel(odds.riskScore);

    auto dist = computeDistribution(odds.winProb, odds.payoutMultiplier,
                                      balance, betAmount, numRounds);
    odds.bustProb = dist.overallBustProb;

    return odds;
}

BetTypeOdds StrategyLab::computeBetOddsForSymbol(
    BetType type, const std::string& symbol,
    double balance, double betAmount, size_t numRounds) {

    BetTypeOdds odds;
    odds.betType = BetTypeToString(type) + "(" + symbol + ")";
    odds.payoutMultiplier = PayoutConfig::getMultiplier(type);

    switch (type) {
        case BetType::EXACT_PREDICTION:
        case BetType::TRIPLE_SYMBOL:
            odds.winProb = calcWinProbTriple(symbol);
            break;
        case BetType::PAIR_PREDICTION:
            odds.winProb = calcWinProbPair(symbol);
            break;
        case BetType::SYMBOL_APPEARANCE:
            odds.winProb = calcWinProbAppearance(symbol);
            break;
        default:
            odds.winProb = 0;
            break;
    }

    odds.expectedValue = odds.winProb * odds.payoutMultiplier;
    odds.variance = odds.payoutMultiplier * odds.payoutMultiplier
                  * odds.winProb * (1.0 - odds.winProb);
    odds.riskScore = 1.0 - odds.winProb;
    odds.riskLevel = riskLevel(odds.riskScore);

    auto dist = computeDistribution(odds.winProb, odds.payoutMultiplier,
                                      balance, betAmount, numRounds);
    odds.bustProb = dist.overallBustProb;

    return odds;
}

std::vector<BetTypeOdds> StrategyLab::computeAllBetOdds(
    double balance, double betAmount, size_t numRounds) {

    std::vector<BetTypeOdds> results;
    auto types = allPredictionTypes();
    for (auto type : types) {
        results.push_back(computeBetOdds(type, balance, betAmount, numRounds));
    }

    // Also add best per-symbol odds for prediction types
    for (auto type : {BetType::SYMBOL_APPEARANCE, BetType::PAIR_PREDICTION,
                      BetType::TRIPLE_SYMBOL, BetType::EXACT_PREDICTION}) {
        for (auto& [sym, p] : symProbs_) {
            results.push_back(computeBetOddsForSymbol(type, sym, balance, betAmount, numRounds));
        }
    }

    std::sort(results.begin(), results.end(),
        [](const auto& a, const auto& b) { return a.expectedValue > b.expectedValue; });
    return results;
}

// ── EV table (symbols × bet types) ────────────────────────────────

std::vector<std::vector<double>> StrategyLab::computeEvTable(
    double balance, double betAmount, size_t numRounds) {

    std::vector<BetType> predTypes = {BetType::EXACT_PREDICTION,
                                       BetType::TRIPLE_SYMBOL,
                                       BetType::PAIR_PREDICTION,
                                       BetType::SYMBOL_APPEARANCE};
    std::vector<std::string> symbols;
    for (auto& [sym, p] : symProbs_) symbols.push_back(sym);

    std::vector<std::vector<double>> table(symbols.size(), std::vector<double>(predTypes.size()));
    for (size_t si = 0; si < symbols.size(); ++si) {
        for (size_t ti = 0; ti < predTypes.size(); ++ti) {
            auto odds = computeBetOddsForSymbol(predTypes[ti], symbols[si],
                                                  balance, betAmount, numRounds);
            table[si][ti] = odds.expectedValue;
        }
    }
    return table;
}

// ── Strategy: Greedy ──────────────────────────────────────────────

StrategyResult StrategyLab::greedyStrategy(double balance, double betAmount, size_t numRounds) {
    StrategyResult result;
    result.name = "Greedy (All-in Highest EV)";

    double bestEV = -1;
    BetType bestType;
    std::string bestSym;

    auto types = allPredictionTypes();
    for (auto type : types) {
        if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL ||
            type == BetType::PAIR_PREDICTION || type == BetType::SYMBOL_APPEARANCE) {
            for (auto& [sym, p] : symProbs_) {
                double wp = 0;
                if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL)
                    wp = calcWinProbTriple(sym);
                else if (type == BetType::PAIR_PREDICTION)
                    wp = calcWinProbPair(sym);
                else
                    wp = calcWinProbAppearance(sym);
                double ev = wp * PayoutConfig::getMultiplier(type);
                if (ev > bestEV) { bestEV = ev; bestType = type; bestSym = sym; }
            }
        } else {
            double wp = (type == BetType::ANY_PAIR) ? calcWinProbAnyPair() : calcWinProbAnyTriple();
            double ev = wp * PayoutConfig::getMultiplier(type);
            if (ev > bestEV) { bestEV = ev; bestType = type; bestSym = ""; }
        }
    }

    std::string label = BetTypeToString(bestType);
    if (!bestSym.empty()) label += "(" + bestSym + ")";
    result.allocations.push_back({label, 1.0, balance});

    BetTypeOdds odds;
    if (bestSym.empty()) {
        odds = computeBetOdds(bestType, balance, betAmount, numRounds);
    } else {
        odds = computeBetOddsForSymbol(bestType, bestSym, balance, betAmount, numRounds);
    }
    result.expectedValue = odds.expectedValue;
    result.expectedBalance = balance * odds.expectedValue;
    result.variance = odds.variance;
    result.bustProbability = odds.bustProb;
    result.riskScore = odds.riskScore;

    return result;
}

// ── Strategy: DP (knapsack allocation across bet types) ────────────

StrategyResult StrategyLab::dpStrategy(double balance, double betAmount,
                                        size_t numRounds, double riskTolerance) {
    StrategyResult result;
    result.name = "DP Optimizer";

    // Build the list of viable (type, symbol) options with their EV and bust prob
    struct Option { std::string label; double ev; double bustProb; };
    std::vector<Option> options;

    auto types = allPredictionTypes();
    for (auto type : types) {
        if (type == BetType::ANY_PAIR || type == BetType::ANY_TRIPLE) {
            double wp = (type == BetType::ANY_PAIR) ? calcWinProbAnyPair() : calcWinProbAnyTriple();
            double mult = PayoutConfig::getMultiplier(type);
            double ev = wp * mult;
            auto dist = computeDistribution(wp, mult, balance, betAmount, numRounds);
            options.push_back({BetTypeToString(type), ev, dist.overallBustProb});
        } else {
            for (auto& [sym, p] : symProbs_) {
                double wp = 0;
                if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL)
                    wp = calcWinProbTriple(sym);
                else if (type == BetType::PAIR_PREDICTION)
                    wp = calcWinProbPair(sym);
                else
                    wp = calcWinProbAppearance(sym);
                double mult = PayoutConfig::getMultiplier(type);
                double ev = wp * mult;
                auto dist = computeDistribution(wp, mult, balance, betAmount, numRounds);
                std::string label = BetTypeToString(type) + "(" + sym + ")";
                options.push_back({label, ev, dist.overallBustProb});
            }
        }
    }

    // Remove dominated options (strictly worse EV and higher bust)
    options.erase(std::remove_if(options.begin(), options.end(),
        [&](const Option& o) {
            for (auto& o2 : options) {
                if (&o == &o2) continue;
                if (o2.ev >= o.ev && o2.bustProb <= o.bustProb) return true;
            }
            return false;
        }), options.end());

    int n = static_cast<int>(options.size());
    int C = 100; // percentage points

    std::vector<double> dp(C + 1, -1e18);
    std::vector<std::vector<std::pair<int, int>>> choice(C + 1);
    dp[0] = 0.0;

    for (int c = 0; c <= C; ++c) {
        if (dp[c] < -1e17) continue;
        for (int i = 0; i < n; ++i) {
            for (int add = 5; c + add <= C; add += 5) {
                double frac = add / 100.0;
                double newEV = dp[c] + frac * options[i].ev;
                double aggBust = options[i].bustProb * frac;
                if (c == 0) aggBust = options[i].bustProb * frac;
                else aggBust = 1.0 - (1.0 - dp[c]) * (1.0 - frac * options[i].bustProb);
                if (aggBust > riskTolerance) continue;
                int nc = c + add;
                if (newEV > dp[nc]) {
                    dp[nc] = newEV;
                    choice[nc] = choice[c];
                    choice[nc].push_back({i, add});
                }
            }
        }
    }

    int bestC = 0;
    for (int c = 0; c <= C; ++c) {
        if (dp[c] > dp[bestC]) bestC = c;
    }

    std::unordered_map<std::string, double> allocMap;
    for (auto& [idx, add] : choice[bestC]) {
        allocMap[options[idx].label] += add / 100.0;
    }

    for (auto& [label, frac] : allocMap) {
        result.allocations.push_back({label, frac, balance * frac});
    }

    result.expectedValue = dp[bestC] / bestC * 100.0;

    // Compute aggregate expected balance
    double aggExp = 0.0;
    double aggBust = 1.0;
    for (auto& [label, frac] : allocMap) {
        for (auto& opt : options) {
            if (opt.label == label) {
                aggExp += frac * opt.ev * balance;
                aggBust *= (1.0 - frac * opt.bustProb);
            }
        }
    }
    result.expectedBalance = aggExp;
    result.bustProbability = 1.0 - aggBust;
    result.riskScore = result.bustProbability;

    return result;
}

// ── Strategy: Kelly Criterion ──────────────────────────────────────

StrategyResult StrategyLab::kellyStrategy(double balance, double betAmount, size_t numRounds) {
    StrategyResult result;
    result.name = "Kelly Criterion";

    double bestKellyFrac = -1;
    BetType bestType;
    std::string bestSym;
    double bestWP = 0;

    auto types = allPredictionTypes();
    for (auto type : types) {
        if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL ||
            type == BetType::PAIR_PREDICTION || type == BetType::SYMBOL_APPEARANCE) {
            for (auto& [sym, p] : symProbs_) {
                double wp = 0;
                if (type == BetType::EXACT_PREDICTION || type == BetType::TRIPLE_SYMBOL)
                    wp = calcWinProbTriple(sym);
                else if (type == BetType::PAIR_PREDICTION)
                    wp = calcWinProbPair(sym);
                else
                    wp = calcWinProbAppearance(sym);
                double mult = PayoutConfig::getMultiplier(type);
                double b = mult - 1.0;
                if (b <= 0) continue;
                double kelly = (wp * b - (1.0 - wp)) / b;
                if (kelly > bestKellyFrac) {
                    bestKellyFrac = kelly;
                    bestType = type;
                    bestSym = sym;
                    bestWP = wp;
                }
            }
        } else {
            double wp = (type == BetType::ANY_PAIR) ? calcWinProbAnyPair() : calcWinProbAnyTriple();
            double mult = PayoutConfig::getMultiplier(type);
            double b = mult - 1.0;
            if (b <= 0) continue;
            double kelly = (wp * b - (1.0 - wp)) / b;
            if (kelly > bestKellyFrac) {
                bestKellyFrac = kelly;
                bestType = type;
                bestSym = "";
                bestWP = wp;
            }
        }
    }

    if (bestKellyFrac < 0) bestKellyFrac = 0.0;

    std::string label = BetTypeToString(bestType);
    if (!bestSym.empty()) label += "(" + bestSym + ")";

    result.allocations.push_back({"Bet " + label, bestKellyFrac, balance * bestKellyFrac});
    result.allocations.push_back({"Hold as Cash", 1.0 - bestKellyFrac, balance * (1.0 - bestKellyFrac)});

    double mult = PayoutConfig::getMultiplier(bestType);
    double ev = bestWP * mult;

    result.expectedValue = ev;
    result.expectedBalance = bestKellyFrac * balance * ev + (1.0 - bestKellyFrac) * balance;
    result.variance = bestKellyFrac * bestKellyFrac * mult * mult * bestWP * (1.0 - bestWP);
    result.bustProbability = 0.0;
    result.riskScore = 1.0 - bestWP;

    return result;
}

// ── Compare all 3 strategies ──────────────────────────────────────

std::vector<StrategyResult> StrategyLab::compareAllStrategies(
    double balance, double betAmount, size_t numRounds, double riskTolerance) {

    StrategyLab lab; // default probs

    // Try to find symbol probabilities from the default weights
    auto defWeights = WeightedReel::defaultWeights();
    int totalW = 0;
    for (auto& [s, w] : defWeights) totalW += w;
    std::unordered_map<std::string, double> probs;
    for (auto& [s, w] : defWeights) probs[s] = static_cast<double>(w) / totalW;
    lab.setSymbolProbabilities(probs);

    std::vector<StrategyResult> results;
    results.push_back(lab.greedyStrategy(balance, betAmount, numRounds));
    results.push_back(lab.dpStrategy(balance, betAmount, numRounds, riskTolerance));
    results.push_back(lab.kellyStrategy(balance, betAmount, numRounds));
    return results;
}
