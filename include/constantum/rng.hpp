#pragma once

#include <cstdint>

namespace constantum {

class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) {
        state_ = seed;
    }
    inline std::uint64_t next() {
        state_ += STEP;
        std::uint64_t output = state_;
        // bijection works for any k: x ^ (x >> k)
        output = ((output >> BIT_SHIFT1) ^ output) * MULT1; // 1
        output = ((output >> BIT_SHIFT2) ^ output) * MULT2; // 2
        output = ((output >> BIT_SHIFT3) ^ output); // 3, no MULT
        return output;
    }

private:
    std::uint64_t state_;
    static constexpr std::uint64_t BIT_SHIFT1 = 30; // based on study -> key is to cover as many digits out of 64 during shifting without overflowing
    static constexpr std::uint64_t BIT_SHIFT2 = 27; // same as above
    static constexpr std::uint64_t BIT_SHIFT3 = 31; // same as above
    static constexpr std::uint64_t STEP = 0x9E3779B97F4A7C15; // based on study -> key is to avoid prime factor of total range (2^64) which is 2, so avoid even nums
    static constexpr std::uint64_t MULT1 = 0xBF58476D1CE4E5B9; // based on study
    static constexpr std::uint64_t MULT2 = 0x94D049BB133111EB; // based on study
};

}  // namespace constantum
