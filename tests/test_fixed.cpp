#include "constantum/fixed.hpp"
#include "test_common.hpp"
#include <stdexcept>

using constantum::Fixed;

void Test_Regression() {
    CHECK(Fixed::FromFloat(1.5f) % Fixed::FromFloat(1.0f) == Fixed::FromFloat(0.5f),
            "1.5 %% 1.0 should be 0.5");
}

constexpr bool ApproxEqual(float a, float b, float eps = 1e-4f) {
    float diff = a - b;
    return (diff < 0 ? -diff : diff) < eps;
}

void Test_RoundTripConsistency() {
    CHECK(ApproxEqual(Fixed::FromFloat(3.25f).Value(), 3.25f), "round-trip 3.25 fails");
}

void Test_NegativeTruncDirectionChange() {
    CHECK(Fixed::FromFloat(-1.5f).Raw() == -6442450944, "Negative truncation direction changed");
}

void Test_DivisionByZeroThrowing() {
    bool is_threw = false;
    try {
        Fixed dummy = Fixed::FromFloat(1.0f) / Fixed::FromFloat(0.0f);
        (void)dummy; // to avoid "unused" warning/error
    } catch (const std::runtime_error&) {
        is_threw = true;
    }
    CHECK(is_threw, "Division by zero fails to throw std::runtime_error");

    is_threw = false;
    try {
        Fixed dummy = Fixed::FromFloat(1.0f) % Fixed::FromFloat(0.0f);
        (void)dummy; // to avoid "unused" warning/error
    } catch (const std::runtime_error&) {
        is_threw = true;
    }
    CHECK(is_threw, "Modulo by zero fails to throw std::runtime_error");

    is_threw = false;
    try {
        Fixed dummy = Fixed::FromFloat(1.0f);
        dummy /= Fixed::FromFloat(0.0f);
    } catch (const std::runtime_error&) {
        is_threw = true;
    }
    CHECK(is_threw, "Division by zero fails to throw std::runtime_error");

    is_threw = false;
    try {
        Fixed dummy = Fixed::FromFloat(1.0f);
        dummy %= Fixed::FromFloat(0.0f);
    } catch (const std::runtime_error&) {
        is_threw = true;
    }
    CHECK(is_threw, "Modulo by zero fails to throw std::runtime_error");
}

void Test_Add() {
    CHECK(Fixed::FromFloat(1.0f) + Fixed::FromFloat(2.0f) == Fixed::FromFloat(3.0f), "1.0 + 2.0 not equal to 3.0");
}

void Test_Minus() {
    CHECK(Fixed::FromFloat(2.0f) - Fixed::FromFloat(1.0f) == Fixed::FromFloat(1.0f), "2.0 - 1.0 not equal to 1.0");
}

void Test_Mult() {
    CHECK(Fixed::FromFloat(2.0f) * Fixed::FromFloat(9.0f) == Fixed::FromFloat(18.0f), "2.0 * 9.0 not equal to 18.0");
}

void Test_Divide() {
    CHECK(Fixed::FromFloat(8.0f) / Fixed::FromFloat(4.0f) == Fixed::FromFloat(2.0f), "8.0 / 4.0 not equal to 2.0");
}

void Test_Modulo() {
    CHECK(Fixed::FromFloat(8.0f) % Fixed::FromFloat(5.0f) == Fixed::FromFloat(3.0f), "8.0 % 5.0 not equal to 3.0");
}

int main() {
    Test_Regression();
    Test_RoundTripConsistency();
    Test_NegativeTruncDirectionChange();
    Test_DivisionByZeroThrowing();
    Test_Add();
    Test_Minus();
    Test_Mult();
    Test_Divide();
    Test_Modulo();
}