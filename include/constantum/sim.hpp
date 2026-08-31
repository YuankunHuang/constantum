#pragma once

#include <concepts>
#include <cstdint>

namespace constantum {

template <typename S>
concept Simulation = requires(S& s, std::uint64_t seed) {
    { s.reset(seed) } -> std::same_as<void>;
    { s.step() } -> std::same_as<void>;
    { s.tick() } -> std::same_as<std::uint64_t>;
    { s.checksum() } -> std::same_as<std::uint64_t>;
};

}  // namespace constantum
