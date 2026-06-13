#include <cstdlib>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "ApiServer.h"
#include "ApplicationController.h"
#include "PasswordHasher.h"

/// Read entire file into a string.
static std::string readFile(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return {};
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

int main() {
    namespace fs = std::filesystem;
    fs::create_directories("data");

    ApplicationController app;
    app.initialise("data");
    app.loadSystem();

    // Seed admin user if no admin exists
    bool hasAdmin = false;
    for (const auto& p : app.getAllPlayers()) {
        if (p.getRole() == "admin") { hasAdmin = true; break; }
    }
    if (!hasAdmin) {
        std::string adminHash = PasswordHasher::hash("admin123");
        app.registerPlayer("ADMIN", "Administrator", "admin", 999999.0,
                           adminHash, 25, "admin");
        std::cout << "[seed] Admin user created: admin / admin123\n";
    }

    // ── Address config ──────────────────────────────────────────────
    // Railway uses PORT, traditional env uses API_PORT
    auto host = std::getenv("API_HOST");
    auto portStr = std::getenv("PORT");
    if (!portStr) portStr = std::getenv("API_PORT");
    auto h = host ? std::string(host) : "0.0.0.0";
    auto p = portStr ? std::stoi(portStr) : 8080;

    ApiServer server(app, h, p);
    auto& svr = server.getServer();

    // ── CORS headers (required when frontend is on Vercel) ──────────
    svr.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"},
    });
    svr.Options(R"(.*)", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204;
    });

    // ── Production static file serving ──────────────────────────────
    // The frontend build (slot-ui/dist) is copied alongside the binary
    // by the Dockerfile.  In dev, run `npm run dev` from slot-ui/.
    for (const auto& distPath : {"slot-ui/dist", "../slot-ui/dist"}) {
        if (fs::exists(distPath)) {
            svr.set_mount_point("/", distPath);
            auto indexHtml = readFile(std::string(distPath) + "/index.html");
            if (!indexHtml.empty()) {
                svr.Get(".*", [indexHtml](const httplib::Request& req,
                                          httplib::Response& res) {
                    // Only catch non-API, non-static routes
                    if (req.path.find("/api/") == 0) return;
                    if (req.path.find(".") != std::string::npos) return;
                    res.set_content(indexHtml, "text/html");
                });
            }
            std::cout << "[static] Mounted " << distPath << "\n";
            break;
        }
    }

    server.run();
    return 0;
}
