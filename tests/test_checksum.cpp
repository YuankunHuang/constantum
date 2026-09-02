#include "constantum/checksum.hpp"
#include "test_common.hpp"
#include <cstdint>
#include <cinttypes>

using constantum::Fnv1a64;

void Test_Determinism() {
    std::uint64_t values[] = {
        0xF0F0F0FFFFFF000F,
        0x0000000000000000,
        0xFFFFFFFFFFFFFFFF,
        0x0F0F0F0F0F0F0F0F,
    };

    for (int i = 0; i < 10000; ++i) {
        Fnv1a64 a{}, b{};
        for (auto v : values) {
            a.Mix(v);
            b.Mix(v);
            CHECK(a.Value() == b.Value(), "Determinism test failed." "v=0x%016" PRIx64 " a.Value=0x%016" PRIx64 " b.Value=0x%016" PRIx64, v, a.Value(), b.Value());
        }
    }
}

void Test_EmptyIsOffsetBasis() {
    Fnv1a64 e{};
    CHECK(e.Value() == 0xcbf29ce484222325, "Empty value not equal to offset basis." "v=0x%016" PRIx64, e.Value());
}

void Test_OrderSensitive() {
    std::uint64_t value1 = 0x0F0F0F0F0F0F0F0F;
    std::uint64_t value2 = 0xF0F0F0F0F0F0F0F0;

    Fnv1a64 ab{}, ba{};
    ab.Mix(value1);
    ab.Mix(value2);
    ba.Mix(value2);
    ba.Mix(value1);
    CHECK(ab.Value() != ba.Value(), "Order sensitive test failed." "ab=0x%016" PRIx64 " ba=0x%016" PRIx64, ab.Value(), ba.Value());
}

int main() {
    Test_Determinism();
    Test_EmptyIsOffsetBasis();
    Test_OrderSensitive();
}