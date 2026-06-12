#pragma once

#include <string>
#include "ApplicationController.h"
#include "BetType.h"
#include "httplib.h"

class ApiServer {
public:
    ApiServer(ApplicationController& app, const std::string& host = "0.0.0.0", int port = 8080);
    void run();
    httplib::Server& getServer() { return svr_; }

private:
    ApplicationController& app_;
    httplib::Server svr_;
    std::string host_;
    int port_;
    int sessionCounter_{};

    void registerRoutes();

    // Auth helpers
    std::string createSession(const std::string& playerId);
    bool validateSession(const httplib::Request& req, std::string& outPlayerId, std::string& outRole);
    bool requireAdmin(const httplib::Request& req, httplib::Response& res);
    bool requireAuth(const httplib::Request& req, httplib::Response& res, std::string& pid, std::string& role);

    // JSON helpers
    static std::string jsonError(const std::string& msg);
};
