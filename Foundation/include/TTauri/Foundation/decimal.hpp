// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/exceptions.hpp"
#include "TTauri/Foundation/int_overflow.hpp"
#include "TTauri/Foundation/math.hpp"
#include <limits>
#include <string_view>
#include <string>
#include <charconv>
#include <ostream.h>

namespace TTauri {

class decimal {
private:
    /* Value contains an 8 bit signed exponent in the most-significant bits
     * and a 56 bit signed mantissa in the least-significant bits.
     */
    uint64_t value;

public:
    constexpr long long mantissa_max = std::numeric_limits<int64_t>::max() >> 8;
    constexpr long long mantissa_min = std::numeric_limits<int64_t>::min() >> 8;
    constexpr int exponent_min = std::numeric_limits<int8_t>::max();
    constexpr int exponent_max = std::numeric_limits<int8_t>::min();

    constexpr decimal(int exponent, long long mantissa) :
        value((static_cast<uint64_t>(exponent) << 56) | ((static_cast<uint64_t>(mantissa) << 8) >> 8))
    {
        if (exponent < exponent_min || exponent > exponent_max) {
            TTAURI_THROW(overflow_error("Overflow of exponent {}", exponent);
        }

        if (mantissa < mantissa_min || mantissa > mantissa_max) {
            TTAURI_THROW(overflow_error("Overflow of mantissa {}", mantissa);
        }
    }

    constexpr decimal(std::pair<int,long long> exponent_mantissa) :
        decimal(exponent_mantissa.first, exponent_mantissa.second) {}

    decimal(std::string_view str) : decimal(to_exponent_mantissa(str)) {}

    constexpr decimal(signed long long x) : decimal(0, x) {}
    constexpr decimal(signed long x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(signed int x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(signed short x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(signed char x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(unsigned long long x) : decimal(0, x) {}
    constexpr decimal(unsigned long x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(unsigned int x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(unsigned short x) : decimal(0, static_cast<long long>(x)) {}
    constexpr decimal(unsigned char x) : decimal(0, static_cast<long long>(x)) {}

    explicit operator signed long long () { return align(0).mantissa(); }
    explicit operator signed long () { return numeric_cast<signed long>(static_cast<signed long long>(*this)); }
    explicit operator signed int () { return numeric_cast<signed int>(static_cast<signed long long>(*this)); }
    explicit operator signed long () { return numeric_cast<signed long>(static_cast<signed long long>(*this)); }
    explicit operator signed int () { return numeric_cast<signed int>(static_cast<signed long long>(*this)); }
    explicit operator signed short () { return numeric_cast<signed int>(static_cast<signed long long>(*this)); }
    explicit operator signed char () { return numeric_cast<signed int>(static_cast<signed long long>(*this)); }
    explicit operator unsigned long long () { return numeric_cast<unsigned int>(static_cast<signed long long>(*this)); }
    explicit operator unsigned long () { return numeric_cast<unsigned long>(static_cast<signed long long>(*this)); }
    explicit operator unsigned int () { return numeric_cast<unsigned int>(static_cast<signed long long>(*this)); }
    explicit operator unsigned long () { return numeric_cast<unsigned long>(static_cast<signed long long>(*this)); }
    explicit operator unsigned int () { return numeric_cast<unsigned int>(static_cast<signed long long>(*this)); }
    explicit operator unsigned short () { return numeric_cast<unsigned int>(static_cast<signed long long>(*this)); }
    explicit operator unsigned char () { return numeric_cast<unsigned int>(static_cast<signed long long>(*this)); }

    explicit operator long double () {
        return static_cast<long double>(mantissa()) * powl(10.0, exponent());
    }
    explicit operator double () { return static_cast<long double>(*this); }
    explicit operator float () { return static_cast<long double>(*this); }

    [[nodiscard]] constexpr int exponent() const noexcept {
        return static_cast<int64_t>(value) >> 56;
    }

    [[nodiscard]] constexpr long long mantissa() const noexcept {
        return static_cast<int64_t>(value << 8) >> 8;
    }

    [[nodiscard]] constexpr std::pair<int,long long> exponent_mantissa() const noexcept {
        return {exponent(), mantissa()};
    }

    /** Return a normalized decimal.
     * The returned decimal will not have trailing zeros.
     *
     * @return A normalized decimal.
     */
    [[nodiscard]] constexpr decimal normalize() const noexcept {
        auto [e, m] = exponent_mantissa();
        if (m != 0) {
            while (m % 10 == 0) {
                m /= 10;
                e++;
            }
        } else {
            e = 0;
        }
        return decimal(e, m);
    }

    /** Return a decimal with the same exponent of another decimal.
     * Shift the mantissa such that exponent of the returned decimal is the same as the
     * other decimal.
     *
     * @return decimal aligned to the other_exponent.
     */
    [[nodiscard]] constexpr decimal align(int other_exponent) const noexcept {
        auto shift = e - other_exponent;
        if (shift == 0) {
            return *this;
        }

        auto [e, m] = exponent_mantissa();
        if (m == 0) {
            return {other_exponent, 0};
        }

        if (shift > 19) {
            TTAURI_THROW(overflow_error("Overflow during alignment of decimal {} to exponent of {}.", *this, other_exponent));

        } else if (shift > 0) {
            long long multiplier = pow10(shift);
            long long r;
            if (mul_overflow(m, multiplier, &r)) {
                TTAURI_THROW(overflow_error("Overflow during alignment of decimal {} to exponent of {}.", *this, other_exponent));
            }

            return {other_exponent, r};

        } else if (shift < -19) {
            return {other_exponent, 0};

        } else {
            long long multiplier = pow10(-shift);
            // Adjust for rounding.
            m += multiplier / 2;
            return {other_exponent, m / multiplier};
        }
    }

    /** Return a decimal with the same exponent of another decimal.
     * Shift the mantissa such that exponent of the returned decimal is the same as the
     * other decimal.
     *
     * @return decimal aligned to the other_exponent.
     */
    [[nodiscard]] constexpr decimal align(decimal other) const noexcept {
        return align(other.exponent());
    }

private:

    [[nodiscard]] std::pair<int,long long> to_exponent_mantissa(std::string_view str) {
        std::string mantissa_str;

        int nr_digits = 0;
        int nr_digits_in_front_of_point = -1;
        for (c: str) {
            if (c >= '0' && c <= '9') {
                mantissa_str += c;
                nr_digits++;
            } else if (c == '.') {
                nr_digits_in_front_of_point = nr_digits;
            } else if (c == '\'' || c == ',') {
                // Ignore thousand separators.
            } else {
                TTAURI_THROW(parse_error("Unexpected character in decimal number '{}'", str));
            }
        }

        int exponent = (nr_digits_in_front_of_point >= 0) ? (nr_digits_in_front_of_point - nr_digits) : 0;

        long long mantissa;
        auto result = std::from_chars(mantissa_str.begin(), mantissa_str.end(), mantissa, 10);
        if (result.ptr == mantissa_str.begin()) {
            TTAURI_THROW(parse_error("Could not parse mantissa '{}'", mantissa_str));
        } else if (result.ec == std::errc::result_out_of_range) {
            TTAURI_THROW(parse_error("Mantissa '{}' out of range ", mantissa_str));
        } else {
            return {exponent, mantissa};
        }
    }

public:
    [[nodiscard]] friend constexpr std::pair<decimal,decimal> align(decimal lhs, decimal rhs, int exponent) {
        return {lhs.align(exponent), rhs.align(exponent)};
    }

    [[nodiscard]] friend constexpr std::pair<decimal,decimal> align(decimal lhs, decimal rhs) {
        return align(lhs, rhs, std::max(lhs.exponent(), rhs.exponent());
    }

    friend constexpr bool operator==(decimal lhs, decimal rhs) {
        auto [lhs_, rhs_] = align(lhs, rhs);
        return lhs_.mantissa() == rhs_.mantissa();
    }

    friend constexpr bool operator<(decimal lhs, decimal rhs) {
        auto [lhs_, rhs_] = align(lhs, rhs);
        return lhs_.mantissa() < rhs_.mantissa();
    }

    friend constexpr bool operator==(decimal lhs, decimal rhs) { return !(lhs == rhs); }
    friend constexpr bool operator>(decimal lhs, decimal rhs) { return rhs < lhs; }
    friend constexpr bool operator<=(decimal lhs, decimal rhs) { return !(lhs > rhs); }
    friend constexpr bool operator>=(decimal lhs, decimal rhs) { return !(lhs < rhs); }

    friend constexpr decimal operator+(decimal lhs, decimal rhs) {
        auto e = std::max(lhs.exponent(), rhs.exponent());
        auto [lhs_, rhs_] = align(lhs, rhs, e);

        long long m;
        if (add_overflow(lhs_.mantissa(), rhs_.mantissa(), &m)) {
            TTAURI_THROW(overflow_error("Overflow when adding mantissas of {} and {}", lhs, rhs));
        }

        return {e, m};
    }

    friend constexpr decimal operator-(decimal lhs, decimal rhs) {
        auto e = std::max(lhs.exponent(), rhs.exponent());
        auto [lhs_, rhs_] = align(lhs, rhs, e);

        long long m;
        if (sub_overflow(lhs_.mantissa(), rhs_.mantissa(), &m)) {
            TTAURI_THROW(overflow_error("Overflow when subtracting mantissas of {} from {}", rhs, lhs));
        }

        return {e, m};
    }

    friend constexpr decimal operator*(decimal lhs, decimal rhs) {
        long long m;
        if (mul_overflow(lhs.mantissa(), rhs.mantissa(), &m)) {
            TTAURI_THROW(overflow_error("Overflow when multiplying mantissas of {} by {}", lhs, rhs));
        }

        auto e = lhs.exponent() + rhs.exponent();
        return {e, m};
    }

    friend constexpr decimal operator/(decimal lhs, decimal rhs) {
        if (rhs.mantissa() == 0) {
            TTAURI_THROW(overflow_error("Divide by zero of {} with {}", lhs, rhs));
        }

        long long m = lhs.mantissa() / rhs.mantissa();
        auto e = lhs.exponent() - rhs.exponent();
        return {e, m};
    }

    friend std::ostream &operator<<(std::ostream &lhs, decimal rhs) {
        return lhs << static_cast<long double>(rhs);
    }
};

}