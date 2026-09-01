#include "constantum/rng.hpp"
#include "test_common.hpp"
#include <cstdint>
#include <cinttypes>

using constantum::SplitMix64;

void TestRng() {

    // we may #include <random> and use std::random_device to
    // create 32-bit random seed and compose a 64-bit seed
    // HOWEVER, if we do that it'll be almost impossible to retrieve that seed
    // and reproduce a test case

    // -> so we use customized manual cases
    constexpr std::uint64_t seeds[] = {
        0xFFFFFFFFFFFFFFFF,
        0x0000000000000000,
        0x0F0F0F0F0F0F0F0F,
        0xF0F0F0F0F0F0F0F0
    }; // stack array, handy and fast

    for (auto seed : seeds) {
        // create two independent RNG (SplitMix64) with the same seed
        SplitMix64 rng1{seed};
        SplitMix64 rng2{seed};

        // generate a certain number of RandomNumbers with both RNGs
        // check if all sub-seeds are equal
        for (int i = 0; i < 10000; ++i) {
            // use custom MACRO
            // we're using std::uint64_t, which is a little bit more complex
            // when formatting
            // -> we need <cinttypes>
            CHECK(rng1.Next() == rng2.Next(), "seed=0x%016" PRIx64 " i=%d", seed, i); 
        }
    }
}

int main() {
    TestRng();
}