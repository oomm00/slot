#include "Reel.h"

Reel::Reel(std::vector<std::string> symbols) : symbols_(std::move(symbols)) {
    if (symbols_.empty()) {
        symbols_ = {"CHERRY", "LEMON"};
    }
}

std::string Reel::spin(RNG& rng) const {
    int idx = rng.nextInt(0, static_cast<int>(symbols_.size()) - 1);
    return symbols_[static_cast<size_t>(idx)];
}

const std::vector<std::string>& Reel::getSymbols() const {
    return symbols_;
}
