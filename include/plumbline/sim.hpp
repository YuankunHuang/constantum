// plumbline/sim.hpp
// The contract every simulation must satisfy to be evaluated by Plumbline.
//
// A simulation is a deterministic state machine stepped in fixed-size ticks.
// The interface is deliberately minimal: anything more, and adding a new
// simulation stops being "an hour of work".
#pragma once

#include <concepts>
#include <cstdint>

namespace plumbline {

// Simulation: a C++20 concept. A conforming type S must:
//   - reset(seed):  (re)start from a clean, fully-seeded state
//   - step():       advance exactly one tick, deterministically
//   - tick():       return the current tick counter
//   - checksum():   return a deterministic checksum of the full state
template <typename S>
concept Simulation = requires(S& s, std::uint64_t seed) {
    { s.reset(seed) } -> std::same_as<void>;
    { s.step() } -> std::same_as<void>;
    { s.tick() } -> std::same_as<std::uint64_t>;
    { s.checksum() } -> std::same_as<std::uint64_t>;
};

}  // namespace plumbline
