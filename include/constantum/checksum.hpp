#pragma once

#include <cstdint>

namespace constantum {

class Fnv1a64 final {
public:
    void Mix(std::uint64_t value) {
        // Mix each and every byte from low to high
        for (int i = 0; i < 8; ++i) {
            std::uint8_t byte = static_cast<std::uint8_t>(value & 0xFF);
            MixByte(byte);
            value >>= 8;
        }
    }
    std::uint64_t Value() const {
        return hash_;
    }

private:
    void MixByte(std::uint8_t byte) {
        hash_ = (hash_ ^ byte) * kFnvPrime;
    }
    std::uint64_t hash_ = kOffsetBasis; // must be initialized - for determinism
    static constexpr std::uint64_t kOffsetBasis = 0xcbf29ce484222325;
    static constexpr std::uint64_t kFnvPrime = 0x100000001b3;
};

}