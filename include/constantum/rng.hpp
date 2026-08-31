#pragma once

#include <cstdint>

namespace constantum {

class SplitMix64 {
public:
    explicit SplitMix64(std::uint64_t seed) {
        state_ = seed;
    }
    inline std::uint64_t Next() {
        state_ += kStep;
        std::uint64_t output = state_;
        // bijection works for any k: x ^ (x >> k)
        output = ((output >> kBitShift1) ^ output) * kMult1; // 1
        output = ((output >> kBitShift2) ^ output) * kMult2; // 2
        output = ((output >> kBitShift3) ^ output); // 3, no MULT
        return output;
    }

private:
    std::uint64_t state_;
    static constexpr std::uint64_t kBitShift1 = 30; // based on study -> key is to cover as many digits out of 64 during shifting without overflowing
    static constexpr std::uint64_t kBitShift2 = 27; // same as above
    static constexpr std::uint64_t kBitShift3 = 31; // same as above
    static constexpr std::uint64_t kStep = 0x9E3779B97F4A7C15; // based on study -> key is to avoid prime factor of total range (2^64) which is 2, so avoid even nums
    static constexpr std::uint64_t kMult1 = 0xBF58476D1CE4E5B9; // based on study
    static constexpr std::uint64_t kMult2 = 0x94D049BB133111EB; // based on study
};

}  // namespace constantum
