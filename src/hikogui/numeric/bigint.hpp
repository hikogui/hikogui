// Copyright Take Vos 2019-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "int_carry.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <format>
#include <type_traits>
#include <ostream>
#include <concepts>

hi_export_module(hikogui.numeric.bigint);

hi_export namespace hi::inline v1 {

/** High performance big integer implementation.
 * The bigint is a fixed width integer which will allow the compiler
 * to make aggressive optimizations, unrolling most loops and easy inlining.
 */
template<std::unsigned_integral DigitType, std::size_t NumDigits, bool IsSigned>
struct bigint {
    using digit_type = DigitType;
    using signed_digit_type = std::make_signed_t<digit_type>;
    constexpr static auto num_digits = NumDigits;
    constexpr static auto is_signed = IsSigned;
    constexpr static auto bits_per_digit = sizeof(digit_type) * CHAR_BIT;

    constexpr static digit_type zero_digit = 0;
    constexpr static digit_type min1_digit = static_cast<digit_type>(signed_digit_type{-1});

    /** Digits, in little endian order.
     */
    digit_type digits[num_digits];

    /** Construct and clear an bigint.
     */
    constexpr bigint() noexcept
    {
        for (std::size_t i = 0; i != num_digits; ++i) {
            digits[i] = zero_digit;
        }
    }

    constexpr bigint(bigint const&) noexcept = default;
    constexpr bigint& operator=(bigint const&) noexcept = default;
    constexpr bigint(bigint&&) noexcept = default;
    constexpr bigint& operator=(bigint&&) noexcept = default;

    /** Construct from a small bigint.
     */
    template<std::size_t N, bool S>
    constexpr bigint(bigint<digit_type, N, S> const& rhs) noexcept
        requires(N < num_digits)
    {
        std::size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != N; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        hilet sign = rhs.is_negative() ? min1_digit : zero_digit;
        for (; i != num_digits; ++i) {
            digits[i] = sign;
        }
    }

    /** Assign from a small bigint.
     */
    template<std::size_t N, bool S>
    constexpr bigint& operator=(bigint<digit_type, N, S> const& rhs) noexcept
        requires(N < num_digits)
    {
        std::size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != N; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        hilet sign = rhs.is_negative() ? min1_digit : zero_digit;
        for (; i != num_digits; ++i) {
            digits[i] = sign;
        }
        return *this;
    }

    constexpr bigint(std::integral auto value) noexcept
    {
        static_assert(sizeof(value) <= sizeof(digit_type));
        static_assert(num_digits > 0);

        if constexpr (std::is_signed_v<decltype(value)>) {
            // Sign extent the value to the size of the first digit.
            digits[0] = truncate<digit_type>(narrow_cast<signed_digit_type>(value));
        } else {
            digits[0] = narrow_cast<digit_type>(value);
        }

        // Sign extent to the rest of the digits.
        hilet sign = value < 0 ? min1_digit : zero_digit;
        for (std::size_t i = 1; i != num_digits; ++i) {
            digits[i] = sign;
        }
    }

    constexpr bigint& operator=(std::integral auto value) noexcept
    {
        static_assert(sizeof(value) <= sizeof(digit_type));
        static_assert(num_digits > 0);

        if constexpr (std::is_signed_v<decltype(value)>) {
            // Sign extent the value to the size of the first digit.
            digits[0] = static_cast<digit_type>(static_cast<signed_digit_type>(value));
        } else {
            digits[0] = static_cast<digit_type>(value);
        }

        // Sign extent to the rest of the digits.
        hilet sign = value < 0 ? min1_digit : zero_digit;
        for (std::size_t i = 1; i != num_digits; ++i) {
            digits[i] = sign;
        }
        return *this;
    }

    constexpr explicit bigint(std::string_view str, int base = 10) : bigint()
    {
        for (hilet c : str) {
            *this *= base;

            if (c >= '0' and c <= '9') {
                *this += char_cast<size_t>(c - '0');
            } else if (c >= 'a' and c <= 'f') {
                *this += char_cast<size_t>(c - 'a' + 10);
            } else if (c >= 'A' and c <= 'F') {
                *this += char_cast<size_t>(c - 'A' + 10);
            } else {
                throw parse_error(std::format("Unexpected character '{}' in string initializing bigint", c));
            }
        }
    }

    constexpr explicit operator unsigned long long() const noexcept
    {
        return truncate<unsigned long long>(digits[0]);
    }
    constexpr explicit operator signed long long() const noexcept
    {
        return truncate<signed long long>(digits[0]);
    }
    constexpr explicit operator unsigned long() const noexcept
    {
        return truncate<unsigned long>(digits[0]);
    }
    constexpr explicit operator signed long() const noexcept
    {
        return truncate<signed long>(digits[0]);
    }
    constexpr explicit operator unsigned int() const noexcept
    {
        return truncate<unsigned int>(digits[0]);
    }
    constexpr explicit operator signed int() const noexcept
    {
        return truncate<signed int>(digits[0]);
    }
    constexpr explicit operator unsigned short() const noexcept
    {
        return truncate<unsigned short>(digits[0]);
    }
    constexpr explicit operator signed short() const noexcept
    {
        return truncate<signed short>(digits[0]);
    }
    constexpr explicit operator unsigned char() const noexcept
    {
        return truncate<unsigned char>(digits[0]);
    }
    constexpr explicit operator signed char() const noexcept
    {
        return truncate<signed char>(digits[0]);
    }

    constexpr explicit operator bool() const noexcept
    {
        for (std::size_t i = 0; i != num_digits; ++i) {
            if (digits[i] != 0) {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] constexpr bool is_negative() const noexcept
    {
        if constexpr (is_signed and num_digits > 0) {
            return static_cast<signed_digit_type>(digits[num_digits - 1]) < 0;
        } else {
            return false;
        }
    }

    template<std::size_t N, bool S>
    constexpr explicit operator bigint<digit_type, N, S>() const noexcept
    {
        auto r = bigint<digit_type, N, S>{};

        hilet sign = is_negative() ? min1_digit : zero_digit;
        for (auto i = 0; i != N; ++i) {
            r.digits[i] = i < num_digits ? digits[i] : sign;
        }
        return r;
    }

    std::string string() const noexcept
    {
        constexpr auto oneOver10 = reciprocal(bigint<digit_type, num_digits * 2, is_signed>{10});

        auto tmp = *this;

        std::string r;

        if (tmp == 0) {
            r = "0";
        } else {
            while (tmp > 0) {
                bigint remainder;
                std::tie(tmp, remainder) = div(tmp, bigint{10}, oneOver10);
                r += (static_cast<unsigned char>(remainder) + '0');
            }
        }

        std::reverse(r.begin(), r.end());
        return r;
    }

    std::string uuid_string() const noexcept
    {
        static_assert(
            std::is_same_v<digit_type, uint64_t> && num_digits == 2,
            "uuid_string should only be called on a uuid compatible type");
        return std::format(
            "{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(digits[1] >> 32),
            static_cast<uint16_t>(digits[1] >> 16),
            static_cast<uint16_t>(digits[1]),
            static_cast<uint16_t>(digits[0] >> 48),
            digits[0] & 0x0000ffff'ffffffffULL);
    }

    [[nodiscard]] constexpr bigint operator-() const noexcept
    {
        bigint r;
        neg_carry_chain(r.digits, digits, num_digits);
        return r;
    }

    constexpr bigint& operator<<=(std::size_t rhs) noexcept
    {
        sll_carry_chain(digits, digits, rhs, num_digits);
        return *this;
    }

    constexpr bigint& operator>>=(std::size_t rhs) noexcept
    {
        if constexpr (is_signed) {
            sra_carry_chain(digits, digits, rhs, num_digits);
        } else {
            srl_carry_chain(digits, digits, rhs, num_digits);
        }
        return *this;
    }

    constexpr bigint& operator*=(bigint const& rhs) noexcept
    {
        auto r = bigint{0};
        mul_carry_chain(r.digits, digits, rhs.digits, num_digits);
        *this = r;
        return *this;
    }

    constexpr bigint& operator+=(bigint const& rhs) noexcept
    {
        add_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint& operator-=(bigint const& rhs) noexcept
    {
        sub_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint& operator&=(bigint const& rhs) noexcept
    {
        and_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint& operator|=(bigint const& rhs) noexcept
    {
        or_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint& operator^=(bigint const& rhs) noexcept
    {
        xor_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    static bigint from_big_endian(uint8_t const *data) noexcept
    {
        hi_axiom_not_null(data);

        auto r = bigint{};
        for (ssize_t i = narrow_cast<ssize_t>(num_digits) - 1; i >= 0; i--) {
            digit_type d = 0;
            for (std::size_t j = 0; j < sizeof(digit_type); j++) {
                d <<= 8;
                d |= *(data++);
            }
            r.digits[i] = d;
        }
        return r;
    }

    static bigint from_little_endian(uint8_t const *data) noexcept
    {
        auto r = bigint{};
        for (int i = 0; i < num_digits; ++i) {
            digit_type d = 0;
            for (std::size_t j = 0; j < sizeof(digit_type); j++) {
                d |= static_cast<digit_type>(*(data++)) << (j * 8);
            }
            r.digits[i] = d;
        }
        return r;
    }

    static bigint from_big_endian(void const *data) noexcept
    {
        return from_big_endian(static_cast<uint8_t const *>(data));
    }

    static bigint from_little_endian(void const *data) noexcept
    {
        return from_little_endian(static_cast<uint8_t const *>(data));
    }

    /** Calculate the remainder of a CRC check.
     * @param lhs The number to check.
     * @param rhs Polynomial.
     * @return the remainder.
     */
    [[nodiscard]] constexpr friend bigint crc(bigint const& lhs, bigint const& rhs) noexcept
        requires(not is_signed)
    {
        hilet polynomialOrder = bsr_carry_chain(rhs.digits, rhs.num_digits);
        hi_assert(polynomialOrder >= 0);

        auto tmp = static_cast<bigint<digit_type, 2 * num_digits, is_signed>>(lhs) << polynomialOrder;
        auto rhs_ = static_cast<bigint<digit_type, 2 * num_digits, is_signed>>(rhs);

        auto tmp_highest_bit = bsr_carry_chain(tmp.digits, tmp.num_digits);
        while (tmp_highest_bit >= polynomialOrder) {
            hilet divident = rhs_ << (tmp_highest_bit - polynomialOrder);

            tmp ^= divident;
            tmp_highest_bit = bsr_carry_chain(tmp.digits, tmp.num_digits);
        }

        return static_cast<bigint>(tmp);
    }

    /** Calculate the reciprocal at a certain precision.
     *
     * N should be two times the size of the eventual numerator.
     *
     * @param rhs The divider of 1.
     * @return (1 << (K*sizeof(T)*8)) / rhs
     */
    [[nodiscard]] constexpr friend bigint reciprocal(bigint const& rhs)
    {
        auto r = bigint<digit_type, num_digits + 1, is_signed>(0);
        r.digits[num_digits] = 1;
        return static_cast<bigint>(r / rhs);
    }

    [[nodiscard]] constexpr friend bool operator==(bigint const& lhs, bigint const& rhs) noexcept
    {
        return eq_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
    }

    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(bigint const& lhs, bigint const& rhs) noexcept
    {
        if constexpr (lhs.is_signed or rhs.is_signed) {
            return cmp_signed_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
        } else {
            return cmp_unsigned_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
        }
    }

    [[nodiscard]] constexpr friend bigint operator<<(bigint const& lhs, std::size_t rhs) noexcept
    {
        auto r = bigint{};
        sll_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator>>(bigint const& lhs, std::size_t rhs) noexcept
    {
        auto r = bigint{};
        if constexpr (lhs.is_signed) {
            sra_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        } else {
            srl_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        }
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator*(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        mul_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator+(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        add_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator-(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        sub_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator~(bigint const& rhs) noexcept
    {
        auto r = bigint{};
        invert_carry_chain(r.digits, rhs.digits, rhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator|(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        or_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator&(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        and_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator^(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto r = bigint{};
        xor_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend std::pair<bigint, bigint> div(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        if constexpr (is_signed) {
            signed_div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        } else {
            div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        }
        return std::pair{quotient, remainder};
    }

    [[nodiscard]] constexpr friend std::pair<bigint, bigint>
    div(bigint const& lhs, bigint const& rhs, bigint<digit_type, 2 * num_digits, is_signed> const& rhs_reciprocal) noexcept
        requires(not is_signed)
    {
        constexpr auto nr_bits = num_digits * bits_per_digit;

        using bigint_x3_type = bigint<digit_type, 3 * num_digits, is_signed>;

        auto quotient = bigint_x3_type{lhs} * bigint_x3_type{rhs_reciprocal};
        quotient >>= (2 * nr_bits);

        auto product = bigint_x3_type{quotient} * bigint_x3_type{rhs};

        hi_axiom(product <= lhs);
        auto remainder = lhs - product;

        int retry = 0;
        while (remainder >= rhs) {
            if (retry++ > 3) {
                return div(lhs, rhs);
            }

            remainder -= rhs;
            quotient += 1;
        }
        return std::pair{static_cast<bigint>(quotient), static_cast<bigint>(remainder)};
    }

    [[nodiscard]] constexpr friend bigint operator/(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        if constexpr (is_signed) {
            signed_div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        } else {
            div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        }
        return quotient;
    }

    [[nodiscard]] constexpr friend bigint operator%(bigint const& lhs, bigint const& rhs) noexcept
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        if constexpr (is_signed) {
            signed_div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        } else {
            div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        }
        return remainder;
    }

    friend std::ostream& operator<<(std::ostream& lhs, bigint const& rhs)
    {
        return lhs << rhs.string();
    }
};

template<std::unsigned_integral T, std::size_t N>
struct is_numeric_unsigned_integral<bigint<T, N, false>> : std::true_type {};

template<std::unsigned_integral T, std::size_t N>
struct is_numeric_signed_integral<bigint<T, N, true>> : std::true_type {};

template<std::unsigned_integral T, std::size_t N, bool S>
struct is_numeric_integral<bigint<T, N, S>> : std::true_type {};

using ubig128 = bigint<uint64_t, 2, false>;
using big128 = bigint<uint64_t, 2, true>;
using uuid = bigint<uint64_t, 2, false>;

} // namespace hi::inline v1

template<std::unsigned_integral DigitType, std::size_t NumDigits, bool IsSigned>
struct std::numeric_limits<hi::bigint<DigitType, NumDigits, IsSigned>> {
    using value_type = hi::bigint<DigitType, NumDigits, IsSigned>;

    constexpr static bool is_specialized = true;
    constexpr static bool is_signed = IsSigned;
    constexpr static bool is_integer = true;
    constexpr static bool is_exact = true;
    constexpr static bool has_infinity = false;
    constexpr static bool has_quiet_NaN = false;
    constexpr static bool has_signaling_NaN = false;
    constexpr static float_round_style round_style = std::round_toward_zero;
    constexpr static bool is_iec559 = false;
    constexpr static bool is_bounded = true;
    constexpr static bool is_modulo = true;
    constexpr static int digits = std::numeric_limits<DigitType>::digits * NumDigits;
    constexpr static int digits10 = std::numeric_limits<DigitType>::digits10 * NumDigits;
    constexpr static int max_digits10 = 0;
    constexpr static int min_exponent = 0;
    constexpr static int min_exponent10 = 0;
    constexpr static int max_exponent = 0;
    constexpr static int max_exponent10 = 0;
    constexpr static bool traps = std::numeric_limits<DigitType>::traps;
    constexpr static bool tinyness_before = false;

    constexpr static value_type min() noexcept
    {
        auto r = value_type{};
        constexpr auto smin = std::numeric_limits<typename value_type::signed_digit_type>::min();
        constexpr auto umin = std::numeric_limits<typename value_type::digit_type>::min();

        for (std::size_t i = 0; i != value_type::num_digits; ++i) {
            r.digits[i] = umin;
        }

        if constexpr (value_type::is_signed and value_type::num_digits > 0) {
            r.digits[value_type::num_digits - 1] = static_cast<typename value_type::digit_type>(smin);
        }

        return r;
    }

    constexpr static value_type lowest() noexcept
    {
        return min();
    }

    constexpr static value_type max() noexcept
    {
        auto r = value_type{};
        constexpr auto smax = std::numeric_limits<typename value_type::signed_digit_type>::max();
        constexpr auto umax = std::numeric_limits<typename value_type::digit_type>::max();

        for (std::size_t i = 0; i != value_type::num_digits; ++i) {
            r.digits[i] = umax;
        }

        if constexpr (value_type::is_signed and value_type::num_digits > 0) {
            r.digits[value_type::num_digits - 1] = smax;
        }

        return r;
    }

    constexpr static value_type epsilon() noexcept
    {
        return value_type{0};
    }

    constexpr static value_type round_error() noexcept
    {
        return value_type{0};
    }

    constexpr static value_type infinity() noexcept
    {
        return value_type{0};
    }

    constexpr static value_type quiet_NaN() noexcept
    {
        return value_type{0};
    }

    constexpr static value_type signaling_NaN() noexcept
    {
        return value_type{0};
    }

    constexpr static value_type denorm_min() noexcept
    {
        return value_type{0};
    }
};
