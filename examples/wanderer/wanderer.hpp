// plumbline/examples/wanderer/wanderer.hpp
// The seed reference simulation: a single deterministic wanderer.
//
// V0 keeps state in int64 and uses the seeded SplitMix64 for movement.
// V1 moves the state to Q16.16 fixed-point and grows this into the
// multi-agent world. Every movement and every bookkeeping value flows
// from the seed, so the same seed reproduces the same checksums exactly.
#pragma once

#include "plumbline/checksum.hpp"
#include "plumbline/rng.hpp"
#include "plumbline/sim.hpp"

#include <cstdint>

namespace plumbline::examples {

class Wanderer {
  public:
    void reset(std::uint64_t seed) {
        seed_ = seed;
        rng_ = SplitMix64(seed);
        tick_ = 0;
        x_ = 0;
        y_ = 0;
        last_draw_ = 0;
    }

    void step() {
        ++tick_;
        const std::int64_t dx = static_cast<std::int64_t>(rng_.next() % 201ULL) - 100;  // [-100, 100]
        const std::int64_t dy = static_cast<std::int64_t>(rng_.next() % 201ULL) - 100;
        x_ += dx;
        y_ += dy;
        last_draw_ = rng_.next();
    }

    std::uint64_t tick() const { return tick_; }
    std::uint64_t checksum() const {
        std::uint64_t h = kFnvOffset;
        h = fold(h, seed_);
        h = fold(h, tick_);
        h = fold(h, static_cast<std::uint64_t>(x_));
        h = fold(h, static_cast<std::uint64_t>(y_));
        h = fold(h, last_draw_);
        return h;
    }

    // Accessors for tests / determinism analysis.
    std::int64_t x() const { return x_; }
    std::int64_t y() const { return y_; }

  private:
    std::uint64_t seed_ = 0;
    SplitMix64 rng_{0};
    std::uint64_t tick_ = 0;
    std::int64_t x_ = 0;
    std::int64_t y_ = 0;
    std::uint64_t last_draw_ = 0;
};

static_assert(Simulation<Wanderer>);

}  // namespace plumbline::examples
