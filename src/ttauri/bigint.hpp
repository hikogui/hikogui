// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "strings.hpp"
#include "math.hpp"
#include "int_carry.hpp"
#include "codec/base_n.hpp"
#include <format>
#include <type_traits>
#include <ostream>

namespace tt {

/** High performance big integer implementation.
 * The bigint is a fixed width integer which will allow the compiler
 * to make aggressive optimizations, unrolling most loops and easy inlining.
 */
template<std::unsigned_integral DigitType, size_t NumDigits, bool IsSigned>
struct bigint {
    using digit_type = DigitType;
    using signed_digit_type = std::make_signed_t<DigitType>;
    static constexpr auto num_digits = NumDigits;
    static constexpr auto is_signed = IsSigned;
    static constexpr auto bits_per_digit = sizeof(digit_type) * CHAR_BIT;

    /** Digits, in little endian order.
     */
    digit_type digits[num_digits];

    /** Construct and clear an bigint.
     */
    constexpr bigint() noexcept
    {
        for (size_t i = 0; i != num_digits; ++i) {
            digits[i] = digit_type{0};
        }
    }

    constexpr bigint(bigint const &) noexcept = default;
    constexpr bigint &operator=(bigint const &) noexcept = default;
    constexpr bigint(bigint &&) noexcept = default;
    constexpr bigint &operator=(bigint &&) noexcept = default;

    /** Construct from a small bigint.
     */
    template<size_t N, bool S>
    constexpr bigint(bigint<digit_type, N, S> const &rhs) noexcept requires(N < num_digits)
    {
        size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != N; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        ttlet sign = rhs.get_sign();
        for (; i != num_digits; ++i) {
            digits[i] = sign;
        }
    }

    /** Assign from a small bigint.
     */
    template<size_t N, bool S>
    constexpr bigint &operator=(bigint<digit_type, N, S> const &rhs) noexcept requires(N < num_digits)
    {
        size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != N; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        ttlet sign = rhs.get_sign();
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
            digits[0] = static_cast<digit_type>(static_cast<signed_digit_type>(value));
        } else {
            digits[0] = static_cast<digit_type>(value);
        }

        // Sign extent to the rest of the digits.
        ttlet sign = get_sign_of_value(value);
        for (size_t i = 1; i != num_digits; ++i) {
            digits[i] = sign;
        }
    }

    constexpr bigint &operator=(std::unsigned_integral auto value) noexcept
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
        ttlet sign = get_sign_of_value(value);
        for (size_t i = 1; i != num_digits; ++i) {
            digits[i] = sign;
        }
        return *this;
    }

    constexpr explicit bigint(std::string_view str, int base = 10) noexcept : bigint()
    {
        size_t i = 0;
        for (; i < str.size(); ++i) {
            (*this) *= base;
            (*this) += base16::int_from_char<int>(str[i]);
        }
    }

    constexpr explicit operator unsigned long long() const noexcept
    {
        return static_cast<unsigned long long>(digits[0]);
    }
    constexpr explicit operator signed long long() const noexcept
    {
        return static_cast<signed long long>(digits[0]);
    }
    constexpr explicit operator unsigned long() const noexcept
    {
        return static_cast<unsigned long>(digits[0]);
    }
    constexpr explicit operator signed long() const noexcept
    {
        return static_cast<signed long>(digits[0]);
    }
    constexpr explicit operator unsigned int() const noexcept
    {
        return static_cast<unsigned int>(digits[0]);
    }
    constexpr explicit operator signed int() const noexcept
    {
        return static_cast<signed int>(digits[0]);
    }
    constexpr explicit operator unsigned short() const noexcept
    {
        return static_cast<unsigned short>(digits[0]);
    }
    constexpr explicit operator signed short() const noexcept
    {
        return static_cast<signed short>(digits[0]);
    }
    constexpr explicit operator unsigned char() const noexcept
    {
        return static_cast<unsigned char>(digits[0]);
    }
    constexpr explicit operator signed char() const noexcept
    {
        return static_cast<signed char>(digits[0]);
    }

    constexpr explicit operator bool() const noexcept
    {
        for (size_t i = 0; i != num_digits; ++i) {
            if (digits[i] != 0) {
                return true;
            }
        }
        return false;
    }

    template<size_t N, bool S>
    constexpr explicit operator bigint<digit_type, N, S>() const noexcept
    {
        auto r = bigint<digit_type, N, S>{};

        ttlet sign = get_sign();
        for (auto i = 0; i != N; ++i) {
            r.digits[i] = i < num_digits ? digits[i] : sign;
        }
        return r;
    }

    std::string string() const noexcept
    {
        static auto oneOver10 = reciprocal(bigint<digit_type, num_digits * 2, is_signed>{10});

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

    std::string UUIDString() const noexcept
    {
        static_assert(
            std::is_same_v<digit_type, uint64_t> && num_digits == 2, "UUIDString should only be called on a uuid compatible type");
        return std::format(
            "{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(digits[1] >> 32),
            static_cast<uint16_t>(digits[1] >> 16),
            static_cast<uint16_t>(digits[1]),
            static_cast<uint16_t>(digits[0] >> 48),
            digits[0] & 0x0000ffff'ffffffffULL);
    }

    /** Get the sign of the integer argument.
     * This returns a digit, that can be used for sign extension.
     * For unsigned values this always returns 0.
     *
     * @return 0 if positive, -1 if negative.
     */
    template<std::integral T>
    [[nodiscard]] constexpr static digit_type get_sign_of_value(T value) noexcept
    {
        if constexpr (std::is_signed_v<T>) {
            return value < 0 ? -1 : 0;

        } else {
            return 0;
        }
    }

    /** Get the sign.
     * This returns a digit, that can be used for sign extension.
     * For unsigned numbers this always returns 0.
     *
     * @return 0 if positive, -1 if negative.
     */
    [[nodiscard]] constexpr digit_type get_sign() const noexcept
    {
        if constexpr (is_signed and num_digits > 0) {
            ttlet last_digit = static_cast<signed_digit_type>(digits[num_digits - 1]);
            // Duplicate the sign bit throughout the whole digit.
            return static_cast<digit_type>(last_digit >> (bits_per_digit - 1));
        } else {
            return 0;
        }
    }

    constexpr bigint &operator<<=(size_t rhs) noexcept
    {
        sll_carry_chain(digits, digits, rhs, num_digits);
        return *this;
    }

    constexpr bigint &operator>>=(size_t rhs) noexcept
    {
        if constexpr (is_signed) {
            sra_carry_chain(digits, digits, rhs, num_digits);
        } else {
            srl_carry_chain(digits, digits, rhs, num_digits);
        }
        return *this;
    }

    constexpr bigint &operator*=(bigint const &rhs) noexcept
    {
        auto r = bigint{0};
        mul_carry_chain(r.digits, digits, rhs.digits, num_digits);
        *this = r;
        return *this;
    }

    constexpr bigint &operator+=(bigint const &rhs) noexcept
    {
        add_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint &operator-=(bigint const &rhs) noexcept
    {
        sub_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint &operator&=(bigint const &rhs) noexcept
    {
        and_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint &operator|=(bigint const &rhs) noexcept
    {
        or_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    constexpr bigint &operator^=(bigint const &rhs) noexcept
    {
        xor_carry_chain(digits, digits, rhs.digits, num_digits);
        return *this;
    }

    static bigint fromBigEndian(uint8_t const *data) noexcept
    {
        auto r = bigint{};
        for (ssize_t i = static_cast<ssize_t>(num_digits) - 1; i >= 0; i--) {
            digit_type d = 0;
            for (size_t j = 0; j < sizeof(digit_type); j++) {
                d <<= 8;
                d |= *(data++);
            }
            r.digits[i] = d;
        }
        return r;
    }
    static bigint fromLittleEndian(uint8_t const *data) noexcept
    {
        auto r = bigint{};
        for (int i = 0; i < num_digits; ++i) {
            digit_type d = 0;
            for (size_t j = 0; j < sizeof(digit_type); j++) {
                d |= static_cast<digit_type>(*(data++)) << (j * 8);
            }
            r.digits[i] = d;
        }
        return r;
    }

    static bigint fromBigEndian(void const *data) noexcept
    {
        return fromBigEndian(static_cast<uint8_t const *>(data));
    }

    static bigint fromLittleEndian(void const *data) noexcept
    {
        return fromLittleEndian(static_cast<uint8_t const *>(data));
    }

    /*! Calculate the remainder of a CRC check.
     * \param r Return value, the remainder.
     * \param lhs The number to check.
     * \param rhs Polynomial.
     */
    [[nodiscard]] friend bigint crc(bigint const &lhs, bigint const &rhs) noexcept requires (not is_signed)
    {
        ttlet polynomialOrder = bsr_carry_chain(rhs.digits, rhs.num_digits);
        tt_assert(polynomialOrder >= 0);

        auto tmp = static_cast<bigint<DigitType, 2 * NumDigits>>(lhs) << polynomialOrder;
        auto rhs_ = static_cast<bigint<DigitType, 2 * NumDigits>>(rhs);

        auto tmp_highest_bit = bsr_carry_chain(tmp.digits, tmp.num_digits);
        while (tmp_highest_bit >= polynomialOrder) {
            ttlet divident = rhs_ << (tmp_highest_bit - polynomialOrder);

            tmp ^= divident;
            tmp_highest_bit = bsr_carry_chain(tmp.digits, tmp.num_digits);
        }

        return static_cast<bigint>(tmp);
    }

    /*! Calculate the reciprocal at a certain precision.
     *
     * N should be two times the size of the eventual numerator.
     *
     * \param divider The divider of 1.
     * \return (1 << (K*sizeof(T)*8)) / divider
     */
    [[nodiscard]] friend bigint reciprocal(bigint const &rhs)
    {
        auto r = bigint<DigitType, NumDigits + 1, IsSigned>(0);
        r.digits[NumDigits] = 1;
        return static_cast<bigint>(r / rhs);
    }

    [[nodiscard]] constexpr friend bool operator==(bigint const &lhs, bigint const &rhs) noexcept
    {
        return eq_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
    }

    [[nodiscard]] constexpr friend std::strong_ordering operator<=>(bigint const &lhs, bigint const &rhs) noexcept
    {
        if constexpr (lhs.is_signed or rhs.is_signed) {
            return cmp_signed_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
        } else {
            return cmp_unsigned_carry_chain(lhs.digits, rhs.digits, lhs.num_digits);
        }
    }

    [[nodiscard]] constexpr friend bigint operator<<(bigint const &lhs, size_t rhs) noexcept
    {
        auto r = bigint{};
        sll_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator>>(bigint const &lhs, size_t rhs) noexcept
    {
        auto r = bigint{};
        if constexpr (lhs.is_signed) {
            sra_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        } else {
            srl_carry_chain(r.digits, lhs.digits, rhs, lhs.num_digits);
        }
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator*(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        mul_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator+(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        add_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator-(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        sub_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator~(bigint const &rhs) noexcept
    {
        auto r = bigint{};
        invert_carry_chain(r.digits, rhs.digits, rhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator|(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        or_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator&(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        and_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend bigint operator^(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto r = bigint{};
        xor_carry_chain(r.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return r;
    }

    [[nodiscard]] constexpr friend std::pair<bigint, bigint> div(bigint const &lhs, bigint const &rhs) noexcept
        requires(not is_signed)
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return std::pair{quotient, remainder};
    }

    [[nodiscard]] constexpr friend std::pair<bigint, bigint>
    div(bigint const &lhs, bigint const &rhs, bigint<DigitType, 2 * NumDigits, IsSigned> const &rhs_reciprocal) noexcept
        requires(not is_signed)
    {
        constexpr auto nr_bits = num_digits * bits_per_digit;

        using bigint_x3_type = bigint<DigitType, 3 * NumDigits, IsSigned>;

        auto quotient = bigint_x3_type{lhs} * bigint_x3_type{rhs_reciprocal};
        quotient >>= (2 * nr_bits);

        auto product = bigint_x3_type{quotient} * bigint_x3_type{rhs};

        tt_axiom(product <= lhs);
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

    [[nodiscard]] constexpr friend bigint operator/(bigint const &lhs, bigint const &rhs) noexcept requires (not is_signed)
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return quotient;
    }

    [[nodiscard]] constexpr friend bigint operator%(bigint const &lhs, bigint const &rhs) noexcept requires(not is_signed)
    {
        auto quotient = bigint{};
        auto remainder = bigint{};

        div_carry_chain(quotient.digits, remainder.digits, lhs.digits, rhs.digits, lhs.num_digits);
        return remainder;
    }

    friend std::ostream &operator<<(std::ostream &lhs, bigint const &rhs)
    {
        return lhs << rhs.string();
    }
};

using ubig128 = bigint<uint64_t, 2, false>;
using uuid = bigint<uint64_t, 2, false>;

} // namespace tt

namespace std {

template<std::unsigned_integral DigitType, size_t NumDigits, bool IsSigned>
struct numeric_limits<bigint<DigitType,NumDigits,IsSigned> {
    static constexpr bool is_specialized = true;
    static constexpr bool is_signed = IsSigned;
    static constexpr bool is_integer = true;
    static constexpr bool is_exact = true;
    static constexpr bool has_infinity = false;
    static constexpr bool has_quiet_NaN = false;
    static constexpr bool has_signaling_NaN = false;
    static constexpr std::float_denorm_style has_denorm = std:denorm_absent;
    static constexpr bool has_denorm_loss = false;
    static constexpr std::float_round_style round_style  std::round_toward_zero;
    static constexpr bool is_iec559 = false;
    static constexpr bool is_bounded = true;
    static constexpr bool is_modulo = true;
    static constexpr int digits = std::numeric_limits<DigitType>::digits * NumDigits;
    static constexpr int digits10 = std::numeric_limits<DigitType>::digits10 * NumDigits;
    static constexpr int max_digits10 = 0;
    static constexpr int min_exponent = 0;
    static constexpr int min_exponent10 = 0;
    static constexpr int max_exponent = 0;
    static constexpr int max_exponent10 = 0;
    static constexpr bool traps = std::numeric_limits<DigitType>::traps;
    static constexpr bool tinyness_before = false;

};


}

