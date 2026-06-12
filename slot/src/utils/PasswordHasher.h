#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include <functional>

struct PasswordHasher {
    static std::string hash(const std::string& password, const std::string& salt = "SlotDAA2024") {
        std::hash<std::string> hasher;
        auto h = hasher(password + salt);
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << h;
        return ss.str();
    }
    static bool verify(const std::string& password, const std::string& hash, const std::string& salt = "SlotDAA2024") {
        return hash == PasswordHasher::hash(password, salt);
    }
};
