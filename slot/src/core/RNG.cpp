#include "RNG.h"

#include <chrono>

RNG::RNG()
    : seed_(static_cast<unsigned>(
          std::chrono::system_clock::now().time_since_epoch().count())) {
    gen_.seed(seed_);
}

RNG::RNG(unsigned seed) : seed_(seed) {
    gen_.seed(seed_);
}

int RNG::nextInt(int min, int max) {
    std::uniform_int_distribution<int> dist(min, max);
    return dist(gen_);
}

unsigned RNG::getSeed() const {
    return seed_;
}
