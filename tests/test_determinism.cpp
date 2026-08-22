// plumbline/tests/test_determinism.cpp
// V0 determinism gate: the same seed must produce bit-identical per-tick
// checksums across two independent runs, on every platform, in CI.
//
// This is the seed of the real gate: later versions run this over a whole
// scenario corpus, with replay, coverage, provenance, and a differential
// oracle. The principle is already here: determinism is proven, not claimed.
#include "plumbline/checksum.hpp"
#include "plumbline/rng.hpp"
#include "plumbline/sim.hpp"

#include "examples/wanderer/wanderer.hpp"

#include <cassert>
#include <cstdio>

namespace {

constexpr std::uint64_t kTicks = 1000;

void test_same_seed_is_bit_identical(std::uint64_t seed) {
    plumbline::examples::Wanderer a;
    plumbline::examples::Wanderer b;
    a.reset(seed);
    b.reset(seed);
    for (std::uint64_t t = 0; t < kTicks; ++t) {
        a.step();
        b.step();
        assert(a.tick() == b.tick());
        assert(a.checksum() == b.checksum());
    }
    std::printf("seed %llu: bit-identical across 2 runs (%llu ticks)  PASS\n",
                static_cast<unsigned long long>(seed),
                static_cast<unsigned long long>(kTicks));
}

void test_different_seeds_diverge() {
    plumbline::examples::Wanderer a;
    plumbline::examples::Wanderer b;
    a.reset(1);
    b.reset(2);
    for (int i = 0; i < 100; ++i) {
        a.step();
        b.step();
    }
    assert(a.checksum() != b.checksum());
    std::printf("different seeds diverge                              PASS\n");
}

}  // namespace

int main() {
    test_same_seed_is_bit_identical(0);
    test_same_seed_is_bit_identical(42);
    test_same_seed_is_bit_identical(0xDEADBEEFULL);
    test_different_seeds_diverge();
    std::printf("\nplumbline V0 determinism smoke test: ALL PASS\n");
    return 0;
}
