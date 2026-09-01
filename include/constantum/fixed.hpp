#pragma once

#include <cstdint>
#include <stdexcept>

namespace constantum {

struct Fixed {
public:

    constexpr Fixed() = default;

    // static creation & initialization
    static constexpr Fixed FromRaw(std::int32_t raw) {
        return Fixed(raw);
    }
    static constexpr Fixed FromFloat(float value) {
        return Fixed(static_cast<std::int32_t>(static_cast<double>(value) * (1 << 16)));
    }

    // arithmetic operators
    constexpr Fixed operator+(Fixed other) const {
        return Fixed(value_ + other.Raw());
    }
    constexpr Fixed operator-(Fixed other) const {
        return Fixed(value_ - other.Raw());
    }
    constexpr Fixed operator*(Fixed other) const {
        return Fixed(static_cast<std::int32_t>((static_cast<std::int64_t>(value_) * other.Raw()) >> 16));
    }
    constexpr Fixed operator/(Fixed other) const {
        if (other.Raw() == 0) {
            throw std::runtime_error("Division by zero");
        }
        return Fixed(static_cast<std::int32_t>((static_cast<std::int64_t>(value_) << 16) / other.Raw()));
    }
    constexpr Fixed operator%(Fixed other) const {
        if (other.Raw() == 0) {
            throw std::runtime_error("Modulo by zero");
        }
        return Fixed(static_cast<std::int32_t>((static_cast<std::int64_t>(value_) << 16) % other.Raw()));
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
        value_ = static_cast<std::int32_t>((static_cast<std::int64_t>(value_) * other.Raw()) >> 16);
        return *this;
    }
    constexpr Fixed& operator/=(Fixed other) {
        if (other.Raw() == 0) {
            throw std::runtime_error("Division by zero");
        }
        value_ = static_cast<std::int32_t>((static_cast<std::int64_t>(value_) << 16) / other.Raw());
        return *this;
    }
    constexpr Fixed& operator%=(Fixed other) {
        if (other.Raw() == 0) {
            throw std::runtime_error("Modulo by zero");
        }
        value_ = static_cast<std::int32_t>((static_cast<std::int64_t>(value_) << 16) % other.Raw());
        return *this;
    }

    // comparison operators
    constexpr auto operator<=>(const Fixed&) const = default;
    constexpr bool operator==(const Fixed&) const = default;
    
    // accessors
    constexpr std::int32_t Raw() const {
        return value_;
    }
    constexpr float Value() const {
        return static_cast<float>(value_) / (1 << 16);
    }

private:
    explicit constexpr Fixed(std::int32_t raw) : value_(raw) {}
    std::int32_t value_ = 0;
};

}