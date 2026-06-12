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

    // Try loading existing data; ignore if none exists.
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

    auto host = std::getenv("API_HOST");
    auto portStr = std::getenv("API_PORT");
    auto h = host ? std::string(host) : "0.0.0.0";
    auto p = portStr ? std::stoi(portStr) : 8080;

    ApiServer server(app, h, p);

    // ── Production static file serving ──────────────────────────────
    // Mount the built frontend for single-binary deployment.
    // During development, use `npm run dev` in slot-ui/ instead.
    auto& svr = server.getServer();

    // Try to mount the dist directory; skip if it doesn't exist.
    if (fs::exists("slot-ui/dist")) {
        svr.set_mount_point("/", "slot-ui/dist");
        auto indexHtml = readFile("slot-ui/dist/index.html");
        if (!indexHtml.empty()) {
            svr.Get(".*", [indexHtml](const httplib::Request&, httplib::Response& res) {
                res.set_content(indexHtml, "text/html");
            });
        }
    }

    server.run();
    return 0;
}
