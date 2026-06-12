#include "PlayerManager.h"

PlayerManager::PlayerManager(CSVStorage storage)
    : storage_(std::move(storage)) {}

bool PlayerManager::addPlayer(const player& p) {
    if (players_.find(p.getplayerid()) != players_.end()) {
        return false;
    }
    players_.emplace(p.getplayerid(), p);
    return true;
}

bool PlayerManager::removePlayer(const std::string& playerId) {
    return players_.erase(playerId) > 0;
}

player* PlayerManager::getPlayer(const std::string& playerId) {
    auto it = players_.find(playerId);
    if (it == players_.end()) return nullptr;
    return &it->second;
}

bool PlayerManager::deposit(const std::string& playerId, double amount) {
    if (amount <= 0.0) return false;
    auto* p = getPlayer(playerId);
    if (!p) return false;
    p->setbal(p->getbal() + amount);
    return true;
}

bool PlayerManager::withdraw(const std::string& playerId, double amount) {
    if (amount <= 0.0) return false;
    auto* p = getPlayer(playerId);
    if (!p) return false;
    if (p->getbal() < amount) return false;
    p->setbal(p->getbal() - amount);
    return true;
}

bool PlayerManager::updateBalance(const std::string& playerId, double balance) {
    if (balance < 0.0) return false;
    auto* p = getPlayer(playerId);
    if (!p) return false;
    p->setbal(balance);
    return true;
}

std::vector<player> PlayerManager::getAllPlayers() const {
    std::vector<player> result;
    result.reserve(players_.size());
    for (const auto& [id, p] : players_) {
        (void)id;
        result.push_back(p);
    }
    return result;
}

bool PlayerManager::loadPlayers() {
    auto loaded = storage_.loadPlayers();
    players_.clear();
    for (const auto& p : loaded) {
        players_.emplace(p.getplayerid(), p);
    }
    return true;
}

bool PlayerManager::savePlayers() {
    return storage_.savePlayers(getAllPlayers());
}
