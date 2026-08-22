// plumbline/rng.hpp
// SplitMix64: a deterministic, seedable PRNG with no global state.
//
// Two instances with the same seed produce identical streams on every
// platform and compiler. This is the foundation of reproducible simulation:
// all randomness in a simulation must come from an explicitly seeded RNG,
// never from wall-clock, entropy, or hash-map iteration order.
#pragma once

#include <cstdint>

namespace plumbline {

class SplitMix64 {
  public:
    explicit SplitMix64(std::uint64_t seed) : state_(seed) {}

    std::uint64_t next() {
        std::uint64_t z = (state_ += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        return z ^ (z >> 31);
    }

  private:
    std::uint64_t state_;
};

}  // namespace plumbline
