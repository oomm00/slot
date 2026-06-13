#include "ApiServer.h"
#include "JsonHelper.h"
#include "PasswordHasher.h"
#include "StrategyLab.h"
#include "BettingAdvisor.h"
#include "WeightedReel.h"

#include <sstream>
#include <unordered_map>

using namespace std;

static unordered_map<string, pair<string, string>> sessions_; // token -> {playerId, role}
static int nextSessionId_ = 1;

string ApiServer::createSession(const string& playerId) {
    const player* p = app_.getPlayerById(playerId);
    if (!p) return "";
    string token = "TOKEN-" + to_string(nextSessionId_++);
    sessions_[token] = {playerId, p->getRole()};
    return token;
}

bool ApiServer::validateSession(const httplib::Request& req, string& outPlayerId, string& outRole) {
    auto authIt = req.headers.find("Authorization");
    if (authIt == req.headers.end()) return false;
    string token = authIt->second;
    if (token.find("Bearer ") == 0) token = token.substr(7);
    auto it = sessions_.find(token);
    if (it == sessions_.end()) return false;
    outPlayerId = it->second.first;
    outRole = it->second.second;
    return true;
}

bool ApiServer::requireAuth(const httplib::Request& req, httplib::Response& res, string& pid, string& role) {
    if (!validateSession(req, pid, role)) {
        res.status = 401;
        res.set_content(jsonError("Unauthorized - valid session required"), "application/json");
        return false;
    }
    return true;
}

bool ApiServer::requireAdmin(const httplib::Request& req, httplib::Response& res) {
    string pid, role;
    if (!requireAuth(req, res, pid, role)) return false;
    if (role != "admin") {
        res.status = 403;
        res.set_content(jsonError("Forbidden - admin access required"), "application/json");
        return false;
    }
    return true;
}

string ApiServer::jsonError(const string& msg) {
    json j = {{"error", msg}};
    return j.dump();
}

ApiServer::ApiServer(ApplicationController& app, const string& host, int port)
    : app_(app), host_(host), port_(port) {
    registerRoutes();
}

void ApiServer::registerRoutes() {
    // ── Auth endpoints ────────────────────────────────────────────
    svr_.Post("/api/register", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            string name = body.value("name", "");
            string username = body.value("username", "");
            string password = body.value("password", "");
            string confirm = body.value("confirmPassword", "");
            int age = body.value("age", 0);

            if (name.empty() || username.empty() || password.empty()) {
                res.status = 400;
                res.set_content(jsonError("name, username, and password are required"), "application/json");
                return;
            }
            if (password != confirm) {
                res.status = 400;
                res.set_content(jsonError("Passwords do not match"), "application/json");
                return;
            }
            if (password.size() < 4) {
                res.status = 400;
                res.set_content(jsonError("Password must be at least 4 characters"), "application/json");
                return;
            }
            if (age < 18) {
                res.status = 400;
                res.set_content(jsonError("Must be at least 18 years old"), "application/json");
                return;
            }

            // Check for duplicate username
            for (const auto& p : app_.getAllPlayers()) {
                if (p.getusername() == username) {
                    res.status = 409;
                    res.set_content(jsonError("Username already taken"), "application/json");
                    return;
                }
            }

            string playerId = "P" + to_string(app_.getAllPlayers().size() + 1);
            string pwHash = PasswordHasher::hash(password);

            if (!app_.registerPlayer(playerId, name, username, 1000.0, pwHash, age, "player")) {
                res.status = 409;
                res.set_content(jsonError("Registration failed"), "application/json");
                return;
            }

            string token = createSession(playerId);
            json j = {{"playerId", playerId}, {"username", username}, {"name", name},
                      {"token", token}, {"balance", 1000.0}, {"role", "player"}};
            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        }
    });

    svr_.Post("/api/login", [this](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            string username = body.value("username", "");
            string password = body.value("password", "");

            if (username.empty() || password.empty()) {
                res.status = 400;
                res.set_content(jsonError("username and password are required"), "application/json");
                return;
            }

            string pwHash = PasswordHasher::hash(password);
            string playerId;
            if (!app_.loginPlayer(username, pwHash, playerId)) {
                res.status = 401;
                res.set_content(jsonError("Invalid credentials"), "application/json");
                return;
            }

            string token = createSession(playerId);
            const player* p = app_.getPlayerById(playerId);
            json j = {{"playerId", playerId}, {"username", p->getusername()}, {"name", p->getname()},
                      {"token", token}, {"balance", p->getbal()}, {"role", p->getRole()}};
            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        }
    });

    // ── Player endpoints (auth required) ──────────────────────────

    svr_.Get("/api/players", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto players = app_.getAllPlayers();
        json j = json::array();
        for (auto& p : players) j.push_back(JsonHelper::playerToJson(p));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get(R"(/api/players/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto targetId = req.matches[1];
        auto p = app_.searchPlayerById(targetId);
        if (!p) {
            res.status = 404;
            res.set_content(jsonError("Player not found"), "application/json");
            return;
        }
        // Public profile for non-admin viewing other players
        if (role == "admin" || targetId == pid) {
            res.set_content(JsonHelper::playerToJson(*p).dump(), "application/json");
        } else {
            json j = JsonHelper::publicProfileToJson(*p);
            res.set_content(j.dump(), "application/json");
        }
    });

    svr_.Put(R"(/api/players/(\w+)/deposit)", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto body = json::parse(req.body);
            auto amount = body.value("amount", 0.0);
            if (amount <= 0) {
                res.status = 400;
                res.set_content(jsonError("Amount must be positive"), "application/json");
                return;
            }
            auto targetId = req.matches[1];
            if (!app_.playerExists(targetId)) {
                res.status = 404;
                res.set_content(jsonError("Player not found"), "application/json");
                return;
            }
            if (targetId != pid && role != "admin") {
                res.status = 403;
                res.set_content(jsonError("Cannot deposit into another player's account"), "application/json");
                return;
            }
            if (!app_.depositFunds(targetId, amount)) {
                res.status = 500;
                res.set_content(jsonError("Deposit failed"), "application/json");
                return;
            }
            json j = {{"playerId", targetId}, {"balance", app_.getBalance(targetId)}};
            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        }
    });

    svr_.Put(R"(/api/players/(\w+)/withdraw)", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto body = json::parse(req.body);
            auto amount = body.value("amount", 0.0);
            if (amount <= 0) {
                res.status = 400;
                res.set_content(jsonError("Amount must be positive"), "application/json");
                return;
            }
            auto targetId = req.matches[1];
            if (!app_.playerExists(targetId)) {
                res.status = 404;
                res.set_content(jsonError("Player not found"), "application/json");
                return;
            }
            if (targetId != pid) {
                res.status = 403;
                res.set_content(jsonError("Cannot withdraw from another player's account"), "application/json");
                return;
            }
            if (!app_.withdrawFunds(targetId, amount)) {
                res.status = 400;
                res.set_content(jsonError("Insufficient funds or withdrawal failed"), "application/json");
                return;
            }
            json j = {{"playerId", targetId}, {"balance", app_.getBalance(targetId)}};
            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        }
    });

    // ── Admin: delete player ──────────────────────────────────────────
    svr_.Delete(R"(/api/players/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        if (role != "admin") {
            res.status = 403;
            res.set_content(jsonError("Forbidden - admin required"), "application/json");
            return;
        }
        auto targetId = req.matches[1];
        if (targetId == pid) {
            res.status = 400;
            res.set_content(jsonError("Cannot delete yourself"), "application/json");
            return;
        }
        if (!app_.playerExists(targetId)) {
            res.status = 404;
            res.set_content(jsonError("Player not found"), "application/json");
            return;
        }
        app_.deletePlayer(targetId);
        json j = {{"success", true}, {"message", "Player deleted"}};
        res.set_content(j.dump(), "application/json");
    });

    // ── Spin endpoint (with bet type and prediction) ──────────────

    svr_.Post("/api/spin", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto body = json::parse(req.body);
            double betAmt = body.value("betAmount", 0.0);
            string betTypeStr = body.value("betType", "any_pair");
            vector<string> prediction;
            if (body.contains("prediction") && body["prediction"].is_array()) {
                for (auto& v : body["prediction"]) prediction.push_back(v.get<string>());
            }

            if (betAmt <= 0) {
                res.status = 400;
                res.set_content(jsonError("betAmount must be > 0"), "application/json");
                return;
            }

            BetType betType = BetTypeFromString(betTypeStr);

            // Validate prediction requirements per bet type
            if ((betType == BetType::EXACT_PREDICTION && prediction.size() != 3) ||
                (betType == BetType::TRIPLE_SYMBOL && prediction.size() != 1) ||
                (betType == BetType::PAIR_PREDICTION && prediction.size() != 1) ||
                (betType == BetType::SYMBOL_APPEARANCE && prediction.size() != 1)) {
                res.status = 400;
                res.set_content(jsonError("Invalid prediction for bet type"), "application/json");
                return;
            }

            auto result = app_.playSpin(pid, betAmt, betType, prediction);
            if (!result.success) {
                res.status = 400;
                res.set_content(jsonError(result.errorMessage), "application/json");
                return;
            }

            json j = JsonHelper::spinResultToJson(result);
            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        }
    });

    // ── Bet types endpoint ────────────────────────────────────────

    svr_.Get("/api/bet-types", [](const httplib::Request&, httplib::Response& res) {
        json j = json::array();
        vector<BetType> types = {BetType::EXACT_PREDICTION, BetType::TRIPLE_SYMBOL,
                                  BetType::PAIR_PREDICTION, BetType::SYMBOL_APPEARANCE,
                                  BetType::ANY_PAIR, BetType::ANY_TRIPLE};
        for (auto bt : types) {
            j.push_back({
                {"type", BetTypeToString(bt)},
                {"multiplier", PayoutConfig::getMultiplier(bt)},
                {"description", getBetTypeDescription(bt)}
            });
        }
        res.set_content(j.dump(), "application/json");
    });

    // ── Player history ────────────────────────────────────────────

    svr_.Get(R"(/api/bets/history/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto targetId = req.matches[1];
        if (targetId != pid && role != "admin") {
            res.status = 403;
            res.set_content(jsonError("Access denied"), "application/json");
            return;
        }
        auto bets = app_.getBettingEngine().getPlayerBets(targetId);
        json j = json::array();
        for (auto& b : bets) j.push_back(JsonHelper::betToJson(b));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get(R"(/api/games/history/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto targetId = req.matches[1];
        if (targetId != pid && role != "admin") {
            res.status = 403;
            res.set_content(jsonError("Access denied"), "application/json");
            return;
        }
        auto games = app_.getGameLogger().getPlayerHistory(targetId);
        json j = json::array();
        for (auto& g : games) j.push_back(JsonHelper::gameRecordToJson(g));
        res.set_content(j.dump(), "application/json");
    });

    // ── Leaderboard ───────────────────────────────────────────────

    svr_.Get("/api/leaderboard", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto n = std::stoul(req.get_param_value("n", 0).empty() ? "10" : req.get_param_value("n"));
        auto players = app_.getLeaderboard(n);
        json j = json::array();
        for (auto& p : players) j.push_back(JsonHelper::playerToJson(p));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get(R"(/api/leaderboard/top/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto metric = req.matches[1];
        auto n = std::stoul(req.get_param_value("n", 0).empty() ? "10" : req.get_param_value("n"));
        vector<player> players;
        if (metric == "balance") players = app_.getTopByBalance(n);
        else if (metric == "winrate") players = app_.getTopByWinRate(n);
        else if (metric == "winnings") players = app_.getTopByTotalWinnings(n);
        else if (metric == "games") players = app_.getTopByGamesPlayed(n);
        else if (metric == "biggestwin") players = app_.getTopByBiggestWin(n);
        else {
            res.status = 400;
            res.set_content(jsonError("Unknown metric"), "application/json");
            return;
        }
        json j = json::array();
        for (auto& p : players) j.push_back(JsonHelper::playerToJson(p));
        res.set_content(j.dump(), "application/json");
    });

    // ── Analytics ─────────────────────────────────────────────────

    svr_.Get(R"(/api/analytics/player/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto targetId = req.matches[1];
        if (targetId != pid && role != "admin") {
            res.status = 403;
            res.set_content(jsonError("Access denied"), "application/json");
            return;
        }
        auto spins = std::stoul(req.get_param_value("spins", 0).empty() ? "100" : req.get_param_value("spins"));
        auto report = app_.generatePlayerReport(targetId, spins);
        auto fraudScore = app_.runFraudCheck(targetId);
        json j = JsonHelper::strategyReportToJson(report);
        j["fraudScore"] = fraudScore;
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get("/api/analytics/system", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto pr = app_.getPerformanceReport();
        json j = {{"houseEdge", pr.houseEdge}, {"overallWinRate", pr.overallWinRate},
                  {"avgStreakLength", pr.avgStreakLength}, {"avgRuinRisk", pr.avgRuinRisk},
                  {"totalPlayers", pr.totalPlayers}, {"totalSpins", pr.totalSpins}};
        res.set_content(j.dump(), "application/json");
    });

    // ── Strategy Lab ────────────────────────────────────────────────

    // Helper: set up StrategyLab with weighted symbol probabilities
    auto makeLab = []() {
        StrategyLab lab;
        auto w = WeightedReel::defaultWeights();
        int total = 0;
        for (auto& [s, wt] : w) total += wt;
        std::unordered_map<std::string, double> probs;
        for (auto& [s, wt] : w) probs[s] = static_cast<double>(wt) / total;
        lab.setSymbolProbabilities(probs);
        return lab;
    };

    svr_.Get("/api/analytics/distribution", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            auto bt = BetTypeFromString(gv("type", "ANY_PAIR"));
            auto lab = makeLab();
            auto r = lab.computeDistributionForType(bt, bal, bet, rounds, gv("symbol", ""));
            json j;
            j["expectedBalances"] = r.expectedBalances;
            j["bustProbabilities"] = r.bustProbabilities;
            j["stdDeviations"] = r.stdDeviations;
            j["overallBustProb"] = r.overallBustProb;
            j["overallSurvivalProb"] = r.overallSurvivalProb;
            j["finalExpectedBalance"] = r.finalExpectedBalance;
            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    svr_.Get("/api/analytics/compare", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            auto lab = makeLab();
            auto rows = lab.computeAllBetOdds(bal, bet, rounds);
            json j = json::array();
            for (auto& r : rows) {
                j.push_back({
                    {"betType", r.betType},
                    {"winProb", r.winProb},
                    {"payoutMultiplier", r.payoutMultiplier},
                    {"expectedValue", r.expectedValue},
                    {"variance", r.variance},
                    {"riskScore", r.riskScore},
                    {"riskLevel", r.riskLevel},
                    {"bustProb", r.bustProb}
                });
            }
            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    svr_.Get("/api/analytics/recommend", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            double riskTol = std::stod(gv("riskTolerance", "0.15"));
            auto lab = makeLab();
            auto dp = lab.dpStrategy(bal, bet, rounds, riskTol);
            json j;
            j["name"] = dp.name;
            j["expectedValue"] = dp.expectedValue;
            j["expectedBalance"] = dp.expectedBalance;
            j["variance"] = dp.variance;
            j["bustProbability"] = dp.bustProbability;
            j["riskScore"] = dp.riskScore;
            j["allocations"] = json::array();
            for (auto& a : dp.allocations) {
                j["allocations"].push_back({
                    {"betType", a.betType},
                    {"fraction", a.fraction},
                    {"credits", a.credits}
                });
            }
            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    svr_.Get("/api/analytics/risk", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            auto bt = BetTypeFromString(gv("type", "ANY_PAIR"));
            auto lab = makeLab();
            auto r = lab.computeDistributionForType(bt, bal, bet, rounds, gv("symbol", ""));
            json j;
            j["overallBustProb"] = r.overallBustProb;
            j["overallSurvivalProb"] = r.overallSurvivalProb;
            j["finalExpectedBalance"] = r.finalExpectedBalance;
            j["balanceStdDev"] = r.stdDeviations.empty() ? 0.0 : r.stdDeviations.back();
            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    // NEW: Compare all 3 strategies side-by-side
    svr_.Get("/api/analytics/strategies", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            double riskTol = std::stod(gv("riskTolerance", "0.15"));
            auto results = StrategyLab::compareAllStrategies(bal, bet, rounds, riskTol);
            json j = json::array();
            for (auto& r : results) {
                json sj;
                sj["name"] = r.name;
                sj["expectedValue"] = r.expectedValue;
                sj["expectedBalance"] = r.expectedBalance;
                sj["variance"] = r.variance;
                sj["bustProbability"] = r.bustProbability;
                sj["riskScore"] = r.riskScore;
                sj["allocations"] = json::array();
                for (auto& a : r.allocations) {
                    sj["allocations"].push_back({
                        {"betType", a.betType},
                        {"fraction", a.fraction},
                        {"credits", a.credits}
                    });
                }
                j.push_back(sj);
            }
            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    // ── Betting Advisor ─────────────────────────────────────────────
    svr_.Get("/api/analytics/advisor", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gv = [&](const string& key, const string& def) -> string {
                auto v = req.get_param_value(key);
                return v.empty() ? def : v;
            };
            double bal = std::stod(gv("balance", "1000"));
            double bet = std::stod(gv("bet", "10"));
            size_t rounds = std::stoul(gv("rounds", "100"));
            double lambdaOverride = -1.0;
            if (req.has_param("lambda")) lambdaOverride = std::stod(gv("lambda", "-1"));

            // Build advisor with same symbol probabilities
            BettingAdvisor adv;
            auto w = WeightedReel::defaultWeights();
            int total = 0;
            for (auto& [s, wt] : w) total += wt;
            std::unordered_map<std::string, double> probs;
            for (auto& [s, wt] : w) probs[s] = static_cast<double>(wt) / total;
            adv.setSymbolProbabilities(probs);

            // Get player bet history for risk profiling
            auto playerBets = app_.getBettingEngine().getPlayerBets(pid);

            auto rec = adv.getRecommendation(playerBets, bal, bet, rounds, lambdaOverride);

            json j;
            j["usedPlayerHistory"] = rec.usedPlayerHistory;

            // Risk profile
            j["riskProfile"] = {
                {"lambda", rec.riskProfile.lambda},
                {"recentWinRate", rec.riskProfile.recentWinRate},
                {"streak", rec.riskProfile.streak},
                {"balanceTrend", rec.riskProfile.balanceTrend},
                {"behaviorLabel", rec.riskProfile.behaviorLabel},
                {"description", rec.riskProfile.description}
            };

            // Top pick
            j["topPick"] = {
                {"label", rec.topPick.label},
                {"betType", rec.topPick.betType},
                {"symbol", rec.topPick.symbol},
                {"winProb", rec.topPick.winProb},
                {"payoutMultiplier", rec.topPick.payoutMultiplier},
                {"expectedValue", rec.topPick.expectedValue},
                {"variance", rec.topPick.variance},
                {"bustProb", rec.topPick.bustProb},
                {"score", rec.topPick.score}
            };

            j["reason"] = rec.reason;

            // Ranked candidates (top 20)
            j["rankedCandidates"] = json::array();
            int count = 0;
            for (auto& c : rec.rankedCandidates) {
                if (count++ >= 20) break;
                j["rankedCandidates"].push_back({
                    {"label", c.label},
                    {"betType", c.betType},
                    {"symbol", c.symbol},
                    {"winProb", c.winProb},
                    {"payoutMultiplier", c.payoutMultiplier},
                    {"expectedValue", c.expectedValue},
                    {"variance", c.variance},
                    {"bustProb", c.bustProb},
                    {"score", c.score}
                });
            }

            res.set_content(j.dump(), "application/json");
        } catch (const exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid parameters: ") + e.what()), "application/json");
        }
    });

    // ── Betting Advisor (POST endpoint with playerId in body) ─────────

    svr_.Post("/api/betting/advise", [this, makeLab](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto body = json::parse(req.body);
            string playerId = body.value("playerId", pid);
            double betAmount = body.value("betAmount", 10.0);

            // Require player to query their own data or be admin
            if (playerId != pid && role != "admin") {
                res.status = 403;
                res.set_content(jsonError("Access denied"), "application/json");
                return;
            }

            // Get player's current balance
            const player* p = app_.getPlayerById(playerId);
            if (!p) {
                res.status = 404;
                res.set_content(jsonError("Player not found"), "application/json");
                return;
            }
            double currentBalance = p->getbal();

            // Build advisor with weighted symbol probabilities
            BettingAdvisor adv;
            auto w = WeightedReel::defaultWeights();
            int total = 0;
            for (auto& [s, wt] : w) total += wt;
            std::unordered_map<std::string, double> probs;
            for (auto& [s, wt] : w) probs[s] = static_cast<double>(wt) / total;
            adv.setSymbolProbabilities(probs);

            // Get player bet history for risk profiling
            auto playerBets = app_.getBettingEngine().getPlayerBets(playerId);

            // Get recommendation (using 10 rounds for DP, no lambda override)
            auto rec = adv.getRecommendation(playerBets, currentBalance, betAmount, 10, -1.0);

            json j;
            j["usedPlayerHistory"] = rec.usedPlayerHistory;

            // Risk profile
            j["riskProfile"] = {
                {"lambda", rec.riskProfile.lambda},
                {"recentWinRate", rec.riskProfile.recentWinRate},
                {"streak", rec.riskProfile.streak},
                {"balanceTrend", rec.riskProfile.balanceTrend},
                {"behaviorLabel", rec.riskProfile.behaviorLabel},
                {"description", rec.riskProfile.description}
            };

            // Recommendation (top pick)
            j["recommendation"] = {
                {"betType", rec.topPick.betType},
                {"symbol", rec.topPick.symbol},
                {"label", rec.topPick.label},
                {"reason", rec.reason}
            };

            j["reason"] = rec.reason;

            // Ranked list (all candidates)
            j["ranked"] = json::array();
            for (auto& c : rec.rankedCandidates) {
                j["ranked"].push_back({
                    {"label", c.label},
                    {"betType", c.betType},
                    {"symbol", c.symbol},
                    {"p_win", c.winProb},
                    {"multiplier", c.payoutMultiplier},
                    {"EV", c.expectedValue},
                    {"bustProbability", c.bustProb},
                    {"variance", c.variance},
                    {"score", c.score}
                });
            }

            // DP Table: compute for top recommendation
            if (!rec.rankedCandidates.empty()) {
                auto& top = rec.topPick;
                // Run a simple 1D DP for visualization
                size_t N = 10;
                size_t M = 50;
                double bucketSize = currentBalance / 25.0;
                
                std::vector<std::vector<double>> dpTable(N + 1, std::vector<double>(M, 0.0));
                size_t initBucket = std::min(static_cast<size_t>(currentBalance / bucketSize), M - 1);
                dpTable[0][initBucket] = 1.0;

                double pWin = top.winProb;
                double pLose = 1.0 - pWin;

                for (size_t round = 0; round < N; ++round) {
                    for (size_t b = 0; b < M; ++b) {
                        double prob = dpTable[round][b];
                        if (prob < 1e-9) continue;

                        if (b == 0) {
                            dpTable[round + 1][0] += prob;
                        } else {
                            double balance = b * bucketSize;
                            double newBalanceWin = balance + betAmount * (top.payoutMultiplier - 1.0);
                            size_t winBucket = std::min(static_cast<size_t>(newBalanceWin / bucketSize), M - 1);
                            dpTable[round + 1][winBucket] += prob * pWin;

                            double newBalanceLose = balance - betAmount;
                            size_t loseBucket = (newBalanceLose > 0) ? std::min(static_cast<size_t>(newBalanceLose / bucketSize), M - 1) : 0;
                            dpTable[round + 1][loseBucket] += prob * pLose;
                        }
                    }
                }

                j["dpTable"] = dpTable;
            }

            res.set_content(j.dump(), "application/json");
        } catch (const json::exception& e) {
            res.status = 400;
            res.set_content(jsonError(string("Invalid JSON: ") + e.what()), "application/json");
        } catch (const exception& e) {
            res.status = 500;
            res.set_content(jsonError(string("Internal error: ") + e.what()), "application/json");
        }
    });

    // ── Fraud ─────────────────────────────────────────────────────

    svr_.Get(R"(/api/fraud/player/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto targetId = req.matches[1];
        if (targetId != pid && role != "admin") {
            res.status = 403;
            res.set_content(jsonError("Access denied"), "application/json");
            return;
        }
        auto score = app_.runFraudCheck(targetId);
        json j = {{"playerId", targetId}, {"fraudScore", score}};
        if (auto* p = app_.searchPlayerById(targetId)) j["flagged"] = p->isfraud();
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get("/api/fraud/alerts", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto stats = app_.getFraudStats();
        json j = {{"totalPlayersScanned", stats.totalPlayersScanned},
                  {"flaggedPlayers", stats.flaggedPlayers},
                  {"averageFraudScore", stats.averageFraudScore},
                  {"highestFraudScore", stats.highestFraudScore}};
        res.set_content(j.dump(), "application/json");
    });

    // ── Search ────────────────────────────────────────────────────

    svr_.Get("/api/search/player", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto q = req.get_param_value("q", 0);
        if (q.empty()) {
            res.status = 400;
            res.set_content(jsonError("Query parameter 'q' is required"), "application/json");
            return;
        }
        auto results = app_.searchPlayerByName(q);
        json j = json::array();
        for (auto& p : results) j.push_back(JsonHelper::playerToJson(p));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get(R"(/api/search/game/(\d+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto gid = std::stoi(req.matches[1]);
            auto game = app_.searchGameById(gid);
            res.set_content(JsonHelper::gameRecordToJson(game).dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(jsonError("Game not found"), "application/json");
        }
    });

    svr_.Get(R"(/api/search/bets/(\w+))", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        try {
            auto bid = req.matches[1];
            auto bet = app_.searchBetById(bid);
            res.set_content(JsonHelper::betToJson(bet).dump(), "application/json");
        } catch (const std::out_of_range&) {
            res.status = 404;
            res.set_content(jsonError("Bet not found"), "application/json");
        }
    });

    svr_.Get("/api/search/date", [this](const httplib::Request& req, httplib::Response& res) {
        string pid, role;
        if (!requireAuth(req, res, pid, role)) return;
        auto start = req.get_param_value("start", 0);
        auto end = req.get_param_value("end", 0);
        if (start.empty() || end.empty()) {
            res.status = 400;
            res.set_content(jsonError("start and end required"), "application/json");
            return;
        }
        auto results = app_.searchGamesByDateRange(start, end);
        json j = json::array();
        for (auto& g : results) j.push_back(JsonHelper::gameRecordToJson(g));
        res.set_content(j.dump(), "application/json");
    });

    // ── Admin endpoints ───────────────────────────────────────────

    svr_.Get("/api/admin/players", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        auto players = app_.getAllPlayers();
        json j = json::array();
        for (auto& p : players) j.push_back(JsonHelper::playerToJson(p));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get("/api/admin/games", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        auto games = app_.getGameLogger().getAll();
        json j = json::array();
        for (auto& g : games) j.push_back(JsonHelper::gameRecordToJson(g));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get("/api/admin/bets", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        auto bets = app_.getBettingEngine().getAllBets();
        json j = json::array();
        for (auto& b : bets) j.push_back(JsonHelper::betToJson(b));
        res.set_content(j.dump(), "application/json");
    });

    svr_.Get("/api/admin/stats", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        auto betStats = app_.getBettingEngine().getStats();
        auto perf = app_.getPerformanceReport();
        auto fraudStats = app_.getFraudStats();
        json j = {
            {"totalBets", betStats.totalBets},
            {"totalWagered", betStats.totalWagered},
            {"totalPayouts", betStats.totalPayouts},
            {"houseProfit", betStats.houseProfit},
            {"winRate", betStats.winRate},
            {"totalPlayers", app_.getAllPlayers().size()},
            {"totalGames", app_.getGameLogger().size()},
            {"fraudStats", {{"flagged", fraudStats.flaggedPlayers},
                           {"avgScore", fraudStats.averageFraudScore}}}
        };
        res.set_content(j.dump(), "application/json");
    });

    // ── Persistence ───────────────────────────────────────────────

    svr_.Post("/api/save", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        if (app_.saveSystem()) res.set_content(R"({"status":"saved"})", "application/json");
        else { res.status = 500; res.set_content(jsonError("Save failed"), "application/json"); }
    });

    svr_.Post("/api/load", [this](const httplib::Request& req, httplib::Response& res) {
        if (!requireAdmin(req, res)) return;
        if (app_.loadSystem()) res.set_content(R"({"status":"loaded"})", "application/json");
        else { res.status = 500; res.set_content(jsonError("Load failed"), "application/json"); }
    });

    // ── Health check ──────────────────────────────────────────────

    svr_.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"ok"})", "application/json");
    });

}

void ApiServer::run() {
    printf("Slot Machine API server starting on %s:%d...\n", host_.c_str(), port_);
    svr_.listen(host_.c_str(), port_);
}
