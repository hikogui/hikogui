// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include "int_overflow.hpp"
#include "math.hpp"
#include "cast.hpp"
#include <limits>
#include <string_view>
#include <string>
#include <charconv>
#include <ostream>
#include <bit>

namespace hi::inline v1 {

class decimal {
private:
    /* Value contains an 8 bit signed exponent in the most-significant bits
     * and a 56 bit signed mantissa in the least-significant bits.
     */
    uint64_t value;

public:
    constexpr static int mantissa_bits = 56;
    constexpr static int exponent_bits = 8;
    constexpr static int exponent_max = 127;
    constexpr static int exponent_min = -128;

    constexpr decimal() noexcept : value(0) {}
    constexpr decimal(decimal const& other) noexcept = default;
    constexpr decimal(decimal&& other) noexcept = default;
    constexpr decimal& operator=(decimal const& other) noexcept = default;
    constexpr decimal& operator=(decimal&& other) noexcept = default;

    constexpr decimal(int exponent, long long mantissa) noexcept : value(decimal::pack(exponent, mantissa)) {}

    constexpr decimal(std::pair<int, long long> exponent_mantissa) : decimal(exponent_mantissa.first, exponent_mantissa.second) {}

    decimal(std::string_view str) : decimal(to_exponent_mantissa(str)) {}
    decimal(double x) noexcept : decimal(to_exponent_mantissa(x)) {}
    decimal(float x) noexcept : decimal(to_exponent_mantissa(x)) {}
    constexpr decimal(signed long long x) : decimal(0, x) {}
    constexpr decimal(signed long x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(signed int x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(signed short x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(signed char x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(unsigned long long x) : decimal(0, x) {}
    constexpr decimal(unsigned long x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(unsigned int x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(unsigned short x) : decimal(0, static_cast<signed long long>(x)) {}
    constexpr decimal(unsigned char x) : decimal(0, static_cast<signed long long>(x)) {}

    constexpr decimal& operator=(std::pair<int, long long> other) noexcept
    {
        value = decimal::pack(other.first, other.second);
        return *this;
    }
    decimal& operator=(std::string_view str) noexcept
    {
        return *this = to_exponent_mantissa(str);
    }
    constexpr decimal& operator=(double other) noexcept
    {
        return *this = to_exponent_mantissa(other);
    }
    constexpr decimal& operator=(float other) noexcept
    {
        return *this = to_exponent_mantissa(other);
    }
    constexpr decimal& operator=(signed long long other) noexcept
    {
        value = decimal::pack(0, other);
        return *this;
    }
    constexpr decimal& operator=(signed long other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(signed int other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(signed short other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(signed char other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(unsigned long long other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(unsigned long other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(unsigned int other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(unsigned short other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }
    constexpr decimal& operator=(unsigned char other) noexcept
    {
        return *this = narrow_cast<signed long long>(other);
    }

    explicit operator signed long long() const noexcept
    {
        auto e = exponent();
        auto m = mantissa();

        while (e < 0) {
            m /= 10;
            e++;
        }

        while (e > 0) {
            m *= 10;
            e--;
            if (!is_valid_mantissa(m)) {
                std::terminate();
            }
        }
        return m;
    }

    explicit operator signed long() const noexcept
    {
        return narrow_cast<signed long>(static_cast<signed long long>(*this));
    }
    explicit operator signed int() const noexcept
    {
        return narrow_cast<signed int>(static_cast<signed long long>(*this));
    }
    explicit operator signed short() const noexcept
    {
        return narrow_cast<signed short>(static_cast<signed long long>(*this));
    }
    explicit operator signed char() const noexcept
    {
        return narrow_cast<signed char>(static_cast<signed long long>(*this));
    }
    explicit operator unsigned long long() const noexcept
    {
        return narrow_cast<unsigned int>(static_cast<signed long long>(*this));
    }
    explicit operator unsigned long() const noexcept
    {
        return narrow_cast<unsigned long>(static_cast<signed long long>(*this));
    }
    explicit operator unsigned int() const noexcept
    {
        return narrow_cast<unsigned int>(static_cast<signed long long>(*this));
    }
    explicit operator unsigned short() const noexcept
    {
        return narrow_cast<unsigned short>(static_cast<signed long long>(*this));
    }
    explicit operator unsigned char() const noexcept
    {
        return narrow_cast<unsigned char>(static_cast<signed long long>(*this));
    }
    explicit operator long double() const noexcept
    {
        return static_cast<long double>(mantissa()) * powl(10.0, exponent());
    }
    explicit operator double() const noexcept
    {
        return static_cast<double>(static_cast<long double>(*this));
    }
    explicit operator float() const noexcept
    {
        return static_cast<float>(static_cast<long double>(*this));
    }

    explicit operator bool() const noexcept
    {
        return mantissa() != 0;
    }

    std::size_t hash() const noexcept
    {
        hilet v = this->normalize();
        return std::hash<uint64_t>{}(v.value);
    }

    /** Extract exponent from value.
     * The exponent is encoded in the least significant bits so that only a
     * MOVSX instruction is needed.
     */
    [[nodiscard]] constexpr int exponent() const noexcept
    {
        return truncate<int8_t>(value);
    }

    /** Extract mantissa from value.
     * The mantissa is encoded in the most significant bits so that only a
     * single SAR instruction with a small shift value is needed.
     */
    [[nodiscard]] constexpr long long mantissa() const noexcept
    {
        return truncate<int64_t>(value) >> exponent_bits;
    }

    [[nodiscard]] constexpr std::pair<int, long long> exponent_mantissa() const noexcept
    {
        return {exponent(), mantissa()};
    }

    /** Return a normalized decimal.
     * The returned decimal will not have trailing zeros.
     *
     * @return A normalized decimal.
     */
    [[nodiscard]] constexpr decimal normalize() const noexcept
    {
        hilet[e, m] = exponent_mantissa();
        hilet[e_, m_] = decimal::normalize(e, m);
        return {e_, m_};
    }

    decimal& operator+=(decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(*this, rhs);
        value = decimal::pack(e, lhs_m + rhs_m);
        return *this;
    }

    decimal& operator-=(decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(*this, rhs);
        value = decimal::pack(e, lhs_m - rhs_m);
        return *this;
    }

    decimal& operator*=(decimal rhs) noexcept
    {
        return *this = *this * rhs;
    }

    decimal& operator/=(decimal rhs) noexcept
    {
        return *this = *this / rhs;
    }

public:
    [[nodiscard]] friend bool operator==(decimal lhs, decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(lhs, rhs);
        return lhs_m == rhs_m;
    }

    [[nodiscard]] friend auto operator<=>(decimal lhs, decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(lhs, rhs);
        return lhs_m <=> rhs_m;
    }

    [[nodiscard]] friend constexpr decimal operator-(decimal rhs) noexcept
    {
        return {rhs.exponent(), -rhs.mantissa()};
    }

    [[nodiscard]] friend decimal operator+(decimal lhs, decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(lhs, rhs);
        return {e, lhs_m + rhs_m};
    }

    [[nodiscard]] friend decimal operator-(decimal lhs, decimal rhs) noexcept
    {
        hilet[e, lhs_m, rhs_m] = decimal::align(lhs, rhs);
        return {e, lhs_m - rhs_m};
    }

    [[nodiscard]] friend decimal operator*(decimal lhs, decimal rhs) noexcept
    {
        auto lhs_e = lhs.exponent();
        auto lhs_m = lhs.mantissa();
        auto rhs_e = rhs.exponent();
        auto rhs_m = rhs.mantissa();

        long long m = 0;
        if (!mul_overflow(lhs_m, rhs_m, &m)) {
            [[likely]] return {lhs_e + rhs_e, m};
        }

        // Try with normalized decimals, this is without loss of precision.
        std::tie(lhs_e, lhs_m) = decimal::normalize(lhs_e, lhs_m);
        std::tie(rhs_e, rhs_m) = decimal::normalize(rhs_e, rhs_m);
        while (mul_overflow(lhs_m, rhs_m, &m)) {
            // Reduce precision of largest mantissa until multiplication succeeds.
            if (lhs_m > rhs_m) {
                lhs_m += 5;
                lhs_m /= 10;
                lhs_e++;
            } else {
                rhs_m += 5;
                rhs_m /= 10;
                rhs_e++;
            }
        }
        return {lhs_e + rhs_e, m};
    }

    [[nodiscard]] friend decimal operator/(decimal lhs, decimal rhs) noexcept
    {
        hilet rhs_m = rhs.mantissa();
        hi_axiom(rhs_m != 0);
        hilet rhs_e = rhs.exponent();
        auto lhs_m = lhs.mantissa();
        auto lhs_e = lhs.exponent();

        std::tie(lhs_e, lhs_m) = decimal::denormalize(lhs_e, lhs_m);
        return {lhs_e - rhs_e, lhs_m / rhs_m};
    }

    [[nodiscard]] friend decimal operator%(decimal lhs, decimal rhs) noexcept
    {
        hilet rhs_m = rhs.mantissa();
        hi_axiom(rhs_m != 0);
        hilet rhs_e = rhs.exponent();
        auto lhs_m = lhs.mantissa();
        auto lhs_e = lhs.exponent();

        std::tie(lhs_e, lhs_m) = decimal::denormalize(lhs_e, lhs_m);
        return {lhs_e - rhs_e, lhs_m % rhs_m};
    }

    [[nodiscard]] friend std::string to_string(decimal x) noexcept
    {
        hilet[e, m] = x.exponent_mantissa();
        auto s = std::to_string(std::abs(m));

        hilet decimal_position = -e;
        hilet leading_zeros = (decimal_position - ssize(s)) + 1;
        if (leading_zeros > 0) {
            s.insert(0, leading_zeros, '0');
        }

        hilet trailing_zeros = e;
        if (trailing_zeros > 0) {
            s.append(trailing_zeros, '0');
        }

        if (decimal_position > 0) {
            s.insert(s.size() - decimal_position, 1, '.');
        }

        if (m < 0) {
            s.insert(0, 1, '-');
        }

        return s;
    }

    friend std::ostream& operator<<(std::ostream& lhs, decimal rhs)
    {
        return lhs << to_string(rhs);
    }

private:
    /** Remove trailing zeros from mantissa.
     */
    [[nodiscard]] constexpr static std::pair<int, long long> normalize(int e, long long m) noexcept
    {
        if (m != 0) {
            while (m % 10 == 0) {
                m /= 10;
                e++;
            }
        }
        return {e, m};
    }

    /** Add trailing zeros to mantissa.
     */
    [[nodiscard]] constexpr static std::pair<int, long long> denormalize(int e, long long m) noexcept
    {
        if (m != 0) {
            // The mantissa is allowed to go slightly over the maximum; since it
            // is used mostly for the rhs of a division, which means the result will,
            // in all probability, make the mantissa smaller than the maximum.
            while (is_valid_mantissa(m)) {
                m *= 10;
                e--;
            }
        }
        return {e, m};
    }

    /*! Check if mantissa is overflowing.
     * @return true if mantissa is valid, false if mantissa is overflowing.
     */
    [[nodiscard]] constexpr static bool is_valid_mantissa(long long m) noexcept
    {
        m >>= (mantissa_bits - 1);
        return m == 0 || m == -1;
    }

    /*! Check if mantissa is overflowing.
     * @return true if mantissa is valid, false if mantissa is overflowing.
     */
    [[nodiscard]] constexpr static bool is_valid_exponent(int e) noexcept
    {
        e >>= (exponent_bits - 1);
        return e == 0 || e == -1;
    }

    [[nodiscard]] static std::tuple<int, long long, long long> align(decimal lhs, decimal rhs) noexcept
    {
        auto lhs_e = lhs.exponent();
        auto lhs_m = lhs.mantissa();
        auto rhs_e = rhs.exponent();
        auto rhs_m = rhs.mantissa();

        if (lhs_e == rhs_e) {
            // No alignment needed.
        } else if (lhs_e > rhs_e) {
            do {
                lhs_m *= 10;
                lhs_e--;
                if (!is_valid_mantissa(lhs_m)) {
                    while (lhs_e > rhs_e) {
                        rhs_m /= 10;
                        rhs_e++;
                    }
                    break;
                }
            } while (lhs_e > rhs_e);
        } else {
            do {
                rhs_m *= 10;
                rhs_e--;
                if (!is_valid_mantissa(lhs_m)) {
                    while (lhs_e < rhs_e) {
                        lhs_m /= 10;
                        lhs_e++;
                    }
                    break;
                }
            } while (lhs_e < rhs_e);
        }
        return {lhs_e, lhs_m, rhs_m};
    }

    /** Pack the exponent and mantissa into a 64 bit unsigned integer.
     */
    [[nodiscard]] constexpr static uint64_t pack(int e, long long m) noexcept
    {
        // Adjust an mantissa that is too large. Precision may be lost.
        while (!is_valid_mantissa(m)) {
            [[unlikely]] m /= 10;
            e++;
            hi_assert(e <= exponent_max);
        }

        while (e > exponent_max) {
            [[unlikely]] if ((m *= 10) == 0)
            {
                e = exponent_max;
                break;
            }
            e--;

            // abort on overflow. This decimal does not support infinite.
            hi_assert(is_valid_mantissa(m));
        }

        while (e < exponent_min) {
            [[unlikely]] if ((m /= 10) == 0)
            {
                e = exponent_min;
                break;
            }
            e++;
        }

        return std::bit_cast<uint64_t>(narrow_cast<int64_t>(m) << exponent_bits) | std::bit_cast<uint8_t>(narrow_cast<int8_t>(e));
    }

    [[nodiscard]] static std::pair<int, long long> to_exponent_mantissa(double x) noexcept
    {
        uint64_t x_;
        std::memcpy(&x_, &x, sizeof(x_));

        auto e2 = narrow_cast<int>((x_ << 1) >> 53) - 1023 - 52;
        auto m = narrow_cast<long long>((x_ << 12) >> 12);
        if (e2 > (-1023 - 52)) {
            // Add leading '1'.
            m |= (1LL << 52);
        }

        if (narrow_cast<int64_t>(x_) < 0) {
            m = -m;
        }

        if (m == 0) {
            return {0, 0};
        }

        int e10 = 0;
        while (e2 < 0) {
            while (is_valid_mantissa(m)) {
                m *= 10;
                e10--;
            }
            m /= 2;
            e2++;
        }

        while (e2 > 0) {
            while (!is_valid_mantissa(m)) {
                m /= 10;
                e10++;
            }
            m *= 2;
            e2--;
        }

        return {e10, m};
    }

    [[nodiscard]] std::pair<int, long long> to_exponent_mantissa(std::string_view str)
    {
        std::string mantissa_str;

        int nr_digits = 0;
        int nr_digits_in_front_of_point = -1;
        for (hilet c : str) {
            if (c >= '0' && c <= '9') {
                mantissa_str += c;
                nr_digits++;
            } else if (c == '.') {
                nr_digits_in_front_of_point = nr_digits;
            } else if (c == '\'' || c == ',') {
                // Ignore thousand separators.
            } else if (c == '-') {
                mantissa_str += c;
            } else {
                throw parse_error(std::format("Unexpected character in decimal number '{}'", str));
            }
        }

        int exponent = (nr_digits_in_front_of_point >= 0) ? (nr_digits_in_front_of_point - nr_digits) : 0;

        auto first = mantissa_str.data();
        auto last = first + mantissa_str.size();
        long long mantissa;
        hilet result = std::from_chars(first, last, mantissa, 10);
        if (result.ptr == first) {
            throw parse_error(std::format("Could not parse mantissa '{}'", mantissa_str));
        } else if (result.ec == std::errc::result_out_of_range) {
            throw parse_error(std::format("Mantissa '{}' out of range ", mantissa_str));
        } else {
            return {exponent, mantissa};
        }
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::decimal> {
    inline std::size_t operator()(hi::decimal const& value) const noexcept
    {
        return value.hash();
    }
};

template<typename CharT>
struct std::formatter<hi::decimal, CharT> : std::formatter<double, CharT> {
    auto format(hi::decimal const& t, auto& fc)
    {
        return std::formatter<double, CharT>::format(static_cast<double>(t), fc);
    }
};
