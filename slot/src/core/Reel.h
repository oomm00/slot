#pragma once

#include <string>
#include <vector>

#include "RNG.h"

/// A single reel holding a pool of possible symbols.
/// On spin() it picks one symbol uniformly at random.
class Reel {
public:
    explicit Reel(std::vector<std::string> symbols);

    /// Pick a random symbol from the pool using the provided RNG.
    std::string spin(RNG& rng) const;

    /// Read-only access to the symbol pool.
    const std::vector<std::string>& getSymbols() const;

private:
    std::vector<std::string> symbols_;
};
