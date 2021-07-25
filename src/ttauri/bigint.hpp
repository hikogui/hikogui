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
template<std::unsigned_integral DigitType, size_t NrDigits, bool IsSigned>
struct bigint {
    using digit_type = DigitType;
    using signed_digit_type = std::make_signed_t<DigitType>;
    static constexpr int nr_digits = NrDigits;
    static constexpr bool is_signed = IsSigned;

    static constexpr int bits_per_digit = sizeof(digit_type) * CHAR_BIT;
    static constexpr int nr_bits = nr_digits * bits_per_digit;

    /** Digits, in little endian order.
     */
    std::array<digit_type,nr_digits> digits;

    /** Construct and clear an bigint.
     */
    constexpr bigint() noexcept {
        for (size_t i = 0; i != nr_digits; ++i) {
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
    constexpr bigint(bigint<digit_type,N,S> const &rhs) noexcept requires(N < nr_digits) {
        size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != N; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        ttlet sign = rhs.get_sign();
        for (; i != nr_digits; ++i) {
            digits[i] = sign;
        }
    }

    /** Assign from a small bigint.
     */
    template<size_t N, bool S>
    constexpr bigint &operator=(bigint<digit_type,N,S> const &rhs) noexcept requires(N < nr_digits) {
        size_t i = 0;

        // Copy the data from a smaller bigint.
        for (; i != O; ++i) {
            digits[i] = rhs.digits[i];
        }

        // Sign extent the most-siginificant-digit.
        ttlet sign = rhs.get_sign();
        for (; i != nr_digits; ++i) {
            digits[i] = sign;
        }
        return *this;
    }

    constexpr bigint(std::integral auto value) noexcept {
        static_assert(sizeof(value) <= sizeof(digit_type));
        static_assert(nr_digits > 0);

        if constexpr (std::is_signed_v<decltype(value)>) {
            // Sign extent the value to the size of the first digit.
            digits[0] = static_cast<digit_type>(static_cast<signed_digit_type>(value));
        } else {
            digits[0] = static_cast<digit_type>(value);
        }

        // Sign extent to the rest of the digits.
        ttlet sign = get_sign_of_value(value);
        for (size_t i = 1; i != nr_digits; ++i) {
            digits[i] = sign;
        }
    }

    constexpr bigint &operator=(std::unsigned_integral auto value) noexcept {
        static_assert(sizeof(value) <= sizeof(digit_type));
        static_assert(nr_digits > 0);

        if constexpr (std::is_signed_v<decltype(value)>) {
            // Sign extent the value to the size of the first digit.
            digits[0] = static_cast<digit_type>(static_cast<signed_digit_type>(value));
        } else {
            digits[0] = static_cast<digit_type>(value);
        }

        // Sign extent to the rest of the digits.
        ttlet sign = get_sign_of_value(value);
        for (size_t i = 1; i != nr_digits; ++i) {
            digits[i] = sign;
        }
        return *this;
    }

    constexpr explicit bigint(std::string_view str, int base=10) noexcept : bigint()
    {
        size_t i = 0;
        for (; i < str.size(); ++i) {
            (*this) *= base;
            (*this) += base16::int_from_char<int>(str[i]);
        }
    }

    constexpr explicit operator unsigned long long () const noexcept { return static_cast<unsigned long long>(digits[0]); }
    constexpr explicit operator signed long long () const noexcept { return static_cast<signed long long>(digits[0]); }
    constexpr explicit operator unsigned long () const noexcept { return static_cast<unsigned long>(digits[0]); }
    constexpr explicit operator signed long () const noexcept { return static_cast<signed long>(digits[0]); }
    constexpr explicit operator unsigned int () const noexcept { return static_cast<unsigned int>(digits[0]); }
    constexpr explicit operator signed int () const noexcept { return static_cast<signed int>(digits[0]); }
    constexpr explicit operator unsigned short () const noexcept { return static_cast<unsigned short>(digits[0]); }
    constexpr explicit operator signed short () const noexcept { return static_cast<signed short>(digits[0]); }
    constexpr explicit operator unsigned char () const noexcept { return static_cast<unsigned char>(digits[0]); }
    constexpr explicit operator signed char () const noexcept { return static_cast<signed char>(digits[0]); }

    constexpr explicit operator bool () const noexcept {
        for (size_t i = 0; i != nr_digits; ++i) {
            if (digits[i] != 0) {
                return true;
            }
        }
        return false;
    }

    template<size_t O>
    constexpr explicit operator bigint<T,O> () const noexcept {
        auto r = bigint<T,O>{};

        for (auto i = 0; i != O; ++i) {
            r.digits[i] = i < nr_digits ? digits[i] : T{0};
        }
        return r;
    }

    std::string string() const noexcept {
        static auto oneOver10 = bigint_reciprocal(bigint<T,N*2>{10});

        auto tmp = *this;

        std::string r;

        if (tmp == 0) {
            r = "0";
        } else {
            while (tmp > 0) {
                bigint remainder;
                std::tie(tmp, remainder) = div(tmp, 10, oneOver10);
                r += (static_cast<char>(remainder) + '0');
            }
        }

        std::reverse(r.begin(), r.end());
        return r;
    }

    std::string UUIDString() const noexcept {
        static_assert(std::is_same_v<T,uint64_t> && N == 2, "UUIDString should only be called on a uuid compatible type");
        return std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(digits[1] >> 32),
            static_cast<uint16_t>(digits[1] >> 16),
            static_cast<uint16_t>(digits[1]),
            static_cast<uint16_t>(digits[0] >> 48),
            digits[0] & 0x0000ffff'ffffffffULL
        );
    }

    /** Get the sign of the integer argument.
     * This returns a digit, that can be used for sign extension.
     * For unsigned values this always returns 0.
     *
     * @return 0 if positive, -1 if negative.
     */
    [[nodiscard]] constexpr static digit_type get_sign_from_value(std::integral auto value) noexcept
    {
        if constexpr (std::is_signed_v<decltype(value)>) {
            ttlet svalue = static_cast<std::make_signed_t<decltype(value)>>(value);
            ttlet svalue_ = static_cast<signed_digit_type>(svalue);
            return static_cast<digit_type>(svalue_ >> (bits_per_digit - 1));

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
        if constexpr (is_signed and nr_digits > 0) {
            ttlet last_digit = static_cast<signed_digit_type>(digits[nr_digits - 1]);
            // Duplicate the sign bit throughout the whole digit.
            return static_cast<digit_type>(last_digit >> (bits_per_digit - 1));
        } else {
            return 0;
        }
    }

    [[nodiscard]] constexpr digit_type get_bit(size_t count) const noexcept {
        tt_axiom(count < nr_digits * bits_per_digit);

        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        return (digits[digit_count] >> bit_count) & 1;
    }

    constexpr void set_bit(size_t count, digit_type value = 1) noexcept {
        tt_axiom(value == 0 || value == 1);
        tt_axiom(count < nr_digits * bits_per_digit);

        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        value <<= bit_count;
        ttlet mask = ~(digit_type{1} << bit_count);
        digits[digit_count] = (digits[digit_count] & mask) | value;
    }

    constexpr bigint &operator<<=(size_t rhs) noexcept {
        bigint_shift_left(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator>>=(size_t rhs) noexcept {
        bigint_shift_right(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator*=(bigint const &rhs) noexcept {
        auto r = bigint{0};
        bigint_multiply(r, *this, rhs);
        *this = r;
        return *this;
    }

    constexpr bigint &operator+=(bigint const &rhs) noexcept {
        bigint_add(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator-=(bigint const &rhs) noexcept {
        bigint_subtract(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator&=(bigint const &rhs) noexcept {
        bigint_and(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator|=(bigint const &rhs) noexcept {
        bigint_or(*this, *this, rhs);
        return *this;
    }

    constexpr bigint &operator^=(bigint const &rhs) noexcept {
        bigint_xor(*this, *this, rhs);
        return *this;
    }

    bigint crc(bigint const &rhs) noexcept {
        return bigint_crc(*this, rhs);
    }

    static bigint fromBigEndian(uint8_t const *data) noexcept {
        auto r = bigint{};
        for (int i = N - 1; i >= 0; i--) {
            digit_type d = 0;
            for (size_t j = 0; j < sizeof(digit_type); j++) {
                d <<= 8;
                d |= *(data++);
            }
            r.digits[i] = d;
        }
        return r;
    }
    static bigint fromLittleEndian(uint8_t const *data) noexcept {
        auto r = bigint{};
        for (int i = 0; i < N; ++i) {
            digit_type d = 0;
            for (size_t j = 0; j < sizeof(digit_type); j++) {
                d |= static_cast<digit_type>(*(data++)) << (j*8);
            }
            r.digits[i] = d;
        }
        return r;
    }

    static bigint fromBigEndian(void const *data) noexcept {
        return fromBigEndian(static_cast<uint8_t const *>(data));
    }

    static bigint fromLittleEndian(void const *data) noexcept {
        return fromLittleEndian(static_cast<uint8_t const *>(data));
    }

    [[nodiscard]] constexpr friend int bigint_compare(bigint const &lhs, bigint const &rhs) noexcept
    {
        for (ssize_t i = N - 1; i >= 0; --i) {
            ttlet lhs_digit = lhs.digits[i];
            ttlet rhs_digit = rhs.digits[i];
            if (lhs_digit != rhs_digit) {
                return lhs_digit < rhs_digit ? -1 : 1;
            }
        }
        return 0;
    }

    friend void bigint_invert(bigint &o, bigint const &rhs) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            o.digits[i] = ~rhs.digits[i];
        }
    }

    friend void bigint_add(bigint &o, bigint const &lhs, bigint const &rhs, T carry=0) noexcept
    {
        for (size_t i = 0; i != N; ++i) {
            std::tie(o.digits[i], carry) = add_carry(lhs.digits[i], rhs.digits[i], carry);
        }
    }

    friend void bigint_multiply(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto rhs_index = 0; rhs_index < N; rhs_index++) {
            ttlet rhs_digit = rhs.digits[rhs_index];

            T carry = 0;
            for (auto lhs_index = 0; (lhs_index + rhs_index) < N; lhs_index++) {
                ttlet lhs_digit = lhs.digits[lhs_index];

                T result;
                T accumulator = o.digits[rhs_index + lhs_index];
                std::tie(result, carry) = mul_carry(lhs_digit, rhs_digit, carry, accumulator);
                o.digits[rhs_index + lhs_index] = result;
            }
        }
    }

    friend void bigint_div(bigint &quotient, bigint &remainder, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = nr_bits - 1; i >= 0; i--) {
            remainder <<= 1;
            remainder |= lhs.get_bit(i);
            if (remainder >= rhs) {
                remainder -= rhs;
                quotient.set_bit(i);
            }
        }
    }

    friend void bigint_div(bigint &r_quotient, bigint &r_remainder, bigint const &lhs, bigint const &rhs, bigint<T,2*N> const &rhs_reciprocal) noexcept
    {
        auto quotient = bigint<T,3*N>{0};
        bigint_multiply(quotient, lhs, rhs_reciprocal);

        quotient >>= (2*nr_bits);

        auto product = bigint<T,3*N>{0};
        bigint_multiply(product, quotient, rhs);

        tt_axiom(product <= lhs);
        auto remainder = lhs - product;

        int retry = 0;
        while (remainder >= rhs) {
            if (retry++ > 3) {
                tt_assert(false);
                bigint_div(r_quotient, r_remainder, lhs, rhs);
            }

            remainder -= rhs;
            quotient += 1;
        }
        r_quotient = static_cast<bigint>(quotient);
        r_remainder = static_cast<bigint>(remainder);
    }

    /*! Bit scan reverse.
    * \return index of leading one, or -1 when rhs is zero.
    */
    [[nodiscard]] friend int bigint_bsr(bigint const &rhs) noexcept
    {
        for (auto i = N - 1; i >= 0; i--) {
            auto tmp = bsr(rhs.digits[i]);
            if (tmp >= 0) {
                return (i * bits_per_digit) + tmp;
            }
        }
        return -1;
    }

    /*! Calculate the remainder of a CRC check.
    * \param r Return value, the remainder.
    * \param lhs The number to check.
    * \param rhs Polynomial.
    */
    [[nodiscard]] friend bigint bigint_crc(bigint const &lhs, bigint const &rhs) noexcept
    {
        ttlet polynomialOrder = bigint_bsr(rhs);
        tt_assert(polynomialOrder >= 0);

        auto tmp = static_cast<bigint<T,2*N>>(lhs) << polynomialOrder;
        auto rhs_ = static_cast<bigint<T,2*N>>(rhs);

        auto tmp_highest_bit = bigint_bsr(tmp);
        while (tmp_highest_bit >= polynomialOrder) {
            ttlet divident = rhs_ << (tmp_highest_bit - polynomialOrder);

            tmp ^= divident;
            tmp_highest_bit = bigint_bsr(tmp);
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
    [[nodiscard]] friend bigint bigint_reciprocal(bigint const &rhs) {
        auto r = bigint<T,N+1>(0);
        r.digits[N] = 1;
        return static_cast<bigint>(r / rhs);
    }


    friend void bigint_and(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] & rhs.digits[i];
        }
    }

    friend void bigint_or(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] | rhs.digits[i];
        }
    }

    friend void bigint_xor(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] ^ rhs.digits[i];
        }
    }

    friend void bigint_subtract(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        bigint rhs_;
        bigint_invert(rhs_, rhs);
        bigint_add(o, lhs, rhs_, T{1});
    }

    friend void bigint_shift_left(bigint &o, bigint const &lhs, int count) noexcept
    {
        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        if (&o != &lhs || digit_count > 0) { 
            for (int i = N - 1; i >= digit_count; --i) {
                o.digits[i] = lhs.digits[i - digit_count];
            }
            for (int i = digit_count - 1; i >= 0; --i) {
                o.digits[i] = 0;
            }
        }

        if (bit_count > 0) {
            T carry = 0;
            for (auto i = 0; i != N; ++i) {
                std::tie(o.digits[i], carry) = shift_left_carry(o.digits[i], bit_count, carry);
            }
        }
    }

    friend void bigint_shift_right(bigint &o, bigint const &lhs, int count) noexcept
    {
        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        if (&o != &lhs || digit_count > 0) { 
            auto i = 0;
            for (; i != (N - digit_count); ++i) {
                o.digits[i] = lhs.digits[narrow_cast<size_t>(i + digit_count)];
            }
            for (; i != N; ++i) {
                o.digits[i] = 0;
            }
        }

        if (bit_count > 0) {
            T carry = 0;
            for (auto i = N - 1; i >= 0; --i) {
                std::tie(o.digits[i], carry) = shift_right_carry(o.digits[i], bit_count, carry);
            }
        }
    }

    [[nodiscard]] friend bool operator==(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) == 0;
    }

    [[nodiscard]] friend bool operator<(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) < 0;
    }

    [[nodiscard]] friend bool operator>(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) > 0;
    }

    [[nodiscard]] friend bool operator!=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) != 0;
    }

    [[nodiscard]] friend bool operator>=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) >= 0;
    }

    [[nodiscard]] friend bool operator<=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) <= 0;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>,int> = 0>
    [[nodiscard]] friend auto operator<<(bigint const &lhs, R const &rhs) noexcept {
        bigint o;
        bigint_shift_left(o, lhs, static_cast<int>(rhs));
        return o;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>,int> = 0>
    [[nodiscard]] friend auto operator>>(bigint const &lhs, R const &rhs) noexcept {
        bigint o;
        bigint_shift_right(o, lhs, static_cast<int>(rhs));
        return o;
    }

    [[nodiscard]] friend auto operator*(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_multiply(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator+(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_add(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator-(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_subtract(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator~(bigint const &rhs) noexcept
    {
        bigint o;
        bigint_invert(o, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator|(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_or(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator&(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_and(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto operator^(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_xor(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] friend auto div(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs);
        return std::pair{ quotient, remainder };
    }

    [[nodiscard]] friend auto div(bigint const &lhs, bigint const &rhs, bigint<T,2*N> const &rhs_reciprocal) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs, rhs_reciprocal);
        return std::pair{ quotient, remainder };
    }


    [[nodiscard]] friend auto operator/(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs);
        return quotient;
    }

    [[nodiscard]] friend auto operator%(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs);
        return remainder;
    }

    friend std::ostream &operator<<(std::ostream &lhs, bigint const &rhs) {
        return lhs << rhs.string();
    }
};

using ubig128 = bigint<uint64_t,2>;
using uuid = bigint<uint64_t,2>;

}
