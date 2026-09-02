#pragma once

#include <cstdint>
#include <stdexcept>

namespace constantum {

// Q32.32 instead of Q16.16, for double precision instead of float precision
// But we can only have up to 64-bit int (std::int64_t) while ensuring cross-platform compatibility
// However, we can "compose" two 64-bit into a new individual data type -- U128
// Unsigned, because we use all 128 digits for number, no room for the sign


struct Fixed {
public:

    constexpr Fixed() = default;

    // static creation & initialization
    static constexpr Fixed FromRaw(std::int64_t raw) {
        return Fixed(raw);
    }
    static constexpr Fixed FromFloat(float value) {
        return Fixed(static_cast<std::int64_t>(static_cast<double>(value) * (1 << 32)));
    }

    // arithmetic operators
    constexpr Fixed operator+(Fixed other) const {
        return Fixed(value_ + other.Raw());
    }
    constexpr Fixed operator-(Fixed other) const {
        return Fixed(value_ - other.Raw());
    }
    constexpr Fixed operator*(Fixed other) const {
        U128 mult = U128::FromMul(value_, other.Raw());

        return Fixed(static_cast<std::int64_t>(mult.Value() >> 32));
    }
    constexpr Fixed operator/(Fixed other) const {
        if (other.Raw() == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Fixed(static_cast<std::int64_t>((static_cast<std::int64_t>(value_) << 32) / other.Raw()));
    }
    constexpr Fixed operator%(Fixed other) const {
        if (other.Raw() == 0) {
            throw std::runtime_error("Modulo by zero");
        }
        return Fixed(value_ % other.Raw());
    }

    // assignment operators
    constexpr Fixed& operator+=(Fixed other) {
        value_ += other.Raw();
        return *this;
    }
    constexpr Fixed& operator-=(Fixed other) {
        value_ -= other.Raw();
        return *this;
    }
    constexpr Fixed& operator*=(Fixed other) {
        value_ = static_cast<std::int64_t>((static_cast<std::int64_t>(value_) * other.Raw()) >> 32);
        return *this;
    }
    constexpr Fixed& operator/=(Fixed other) {
        if (other.Raw() == 0) {
            throw std::runtime_error("Division by zero");
        }
        value_ = static_cast<std::int64_t>((static_cast<std::int64_t>(value_) << 32) / other.Raw());
        return *this;
    }
    constexpr Fixed& operator%=(Fixed other) {
        if (other.Raw() == 0) {
            throw std::runtime_error("Modulo by zero");
        }
        value_ %= other.Raw();
        return *this;
    }

    // comparison operators
    constexpr auto operator<=>(const Fixed&) const = default;
    constexpr bool operator==(const Fixed&) const = default;
    
    // accessors
    constexpr std::int64_t Raw() const {
        return value_;
    }
    constexpr float Value() const {
        return static_cast<float>(value_) / (1 << 32);
    }

private:
    struct U128 {
    private:
        U128(std::uint64_t high, std::uint64_t low) : hi(high), lo(low) {}    

    public:
        std::uint64_t hi;
        std::uint64_t lo;
        // value = hi * 2^64 (1<<64) + lo

        /* The ONLY Public Constructor -> Factory */
        static U128 FromMul(std::uint64_t a, std::uint64_t b) {

        }

        /* For Long Division*/
        static int Compare(const U128& a, const U128& b) {
            if (a.hi != b.hi) {
                if (a.hi < b.hi) return -1;
                return 1;
            }
            if (a.lo != b.lo) {
                if (a.lo < b.lo) return -1;
                return 1;
            }
            return 0;
        }
        static U128 Sub(const U128& a, const U128& b) {

        }
        static U128 ShiftLeft1(const U128& a) {
            return U128{a.hi << 1, b.hi << 1};
        }

        constexpr U128 operator*(const U128& other) const {
            std::uint64_t a_lo = a & FFFFFFFFu, a_hi = a >> 32;
            std::uint64_t b_lo = b & FFFFFFFFu, b_hi = b >> 32;
            // a * b = (a_hi * 2^32 + a_lo) * (b_hi * 2^32 + b_lo)
            // a * b = a_hi * b_hi * 2^64 + 2^32 * (a_hi * b_lo + a_lo * b_hi) + a_lo * b_lo
            std::uint64_t hi_hi = a_hi * b_hi;
            std::uint64_t hi_lo = a_hi * b_lo;
            std::uint64_t lo_hi = a_lo * b_hi;
            std::uint64_t lo_lo = a_lo * b_lo;

            std::uint64_t low = a_lo * b_lo;
            std::uint64_t high = a_hi * b_hi * (1 << 32) + a_hi * b_lo + a_lo * b_hi;
            return U128{high, low};
        }
    }

    explicit constexpr Fixed(std::int64_t raw) : value_(raw) {}
    std::uint64_t value_ = 0;
};

}