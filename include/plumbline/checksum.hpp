// plumbline/checksum.hpp
// A deterministic, order-sensitive state fold (FNV-1a mixing).
//
// Determinism is defined as: same seed + same scenario → identical state
// checksum at every tick. The checksum must depend on the ORDER of the
// fields folded in, so that any reordering of state changes the result.
#pragma once

#include <cstdint>

namespace plumbline {

// FNV-1a offset basis (64-bit).
inline constexpr std::uint64_t kFnvOffset = 0xCBF29CE484222325ULL;
// FNV-1a prime (64-bit).
inline constexpr std::uint64_t kFnvPrime = 0x100000001B3ULL;

// Fold one 64-bit value into an accumulator. Deterministic by construction:
// pure integer arithmetic, no floating point, no allocation, no globals.
inline std::uint64_t fold(std::uint64_t state, std::uint64_t value) {
    state ^= value;
    state *= kFnvPrime;
    return state;
}

}  // namespace plumbline
