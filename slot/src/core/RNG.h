#pragma once

#include <random>

/// Wrapper around std::mt19937 for reproducible randomness.
class RNG {
public:
    RNG();
    explicit RNG(unsigned seed);

    /// Returns a random integer in [min, max] inclusive.
    int nextInt(int min, int max);

    /// The seed used to initialise the generator.
    unsigned getSeed() const;

private:
    std::mt19937 gen_;
    unsigned seed_;
};
