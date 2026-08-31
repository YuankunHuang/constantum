#pragma once

#include <concepts>
#include <cstdint>

namespace constantum {

template <typename S>
concept Simulation = requires(S& s, std::uint64_t seed) {
    { s.Reset(seed) } -> std::same_as<void>;
    { s.Step() } -> std::same_as<void>;
    { s.Tick() } -> std::same_as<std::uint64_t>;
    { s.Checksum() } -> std::same_as<std::uint64_t>;
};

}  // namespace constantum
