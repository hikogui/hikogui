// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/strings.hpp"
#include "ttauri/math.hpp"
#include "ttauri/int_carry.hpp"
#include <fmt/format.h>
#include <type_traits>
#include <ostream>

namespace tt {

//template<typename T, int N, bool SIGNED=false>
//struct bigint;

//template<typename T, int N, int O>
//tt_force_inline bigint<T,N> bigint_reciprocal(bigint<T,O> const &divider);

//template<typename T, int N, typename U>
//constexpr bigint<T,N> bigint_reciprocal(U const &divider);

/*! High performance big integer implementation.
 * The bigint is a fixed width integer which will allow the compiler
 * to make aggressive optimizations, unrolling most loops and easy inlining.
 */
template<typename T, int N>
struct bigint {
    static_assert(N >= 0, "bigint must have zero or more digits.");
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "bigint's digit must be unsigned integers.");

    using digit_type = T;
    static constexpr int nr_digits = N;
    static constexpr int bits_per_digit = sizeof(digit_type) * 8;
    static constexpr int nr_bits = nr_digits * bits_per_digit;

    /*! Digits, in little endian order.
    */
    std::array<digit_type,nr_digits> digits;

    tt_force_inline bigint() noexcept = default;
    tt_force_inline ~bigint() = default;
    tt_force_inline bigint(bigint const &) noexcept = default;
    tt_force_inline bigint &operator=(bigint const &) noexcept = default;
    tt_force_inline bigint(bigint &&) noexcept = default;
    tt_force_inline bigint &operator=(bigint &&) noexcept = default;

    template<int R, std::enable_if_t<R < N, int> = 0>
    tt_force_inline bigint(bigint<T,R> const &rhs) noexcept {
        int i = 0;
        for (; i != R; ++i) {
            digits[i] = rhs.digits[i];
        }
        for (; i != N; ++i) {
            digits[i] = 0;
        }
    }

    template<int R, std::enable_if_t<R < N, int> = 0>
    tt_force_inline bigint &operator=(bigint<T,R> const &rhs) noexcept {
        int i = 0;
        for (; i != R; ++i) {
            digits[i] = rhs.digits[i];
        }
        for (; i != N; ++i) {
            digits[i] = 0;
        }
        return *this;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>, int> = 0>
    tt_force_inline bigint(R value) noexcept {
        digits[0] = static_cast<digit_type>(value);
        for (auto i = 1; i < nr_digits; i++) {
            digits[i] = 0;
        }
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>, int> = 0>
    tt_force_inline bigint &operator=(R value) noexcept {
        digits[0] = value;
        for (auto i = 1; i < nr_digits; i++) {
            digits[i] = 0;
        }
        return *this;
    }

    tt_force_inline explicit bigint(std::string_view str, int base=10) noexcept :
        bigint(0)
    {
        for (auto i = 0; i < str.size(); i++) {
            auto nibble = char_to_nibble(str[i]);
            (*this) *= base;
            (*this) += nibble;
        }
    }

    tt_force_inline explicit operator unsigned long long () const noexcept { return static_cast<unsigned long long>(digits[0]); }
    tt_force_inline explicit operator signed long long () const noexcept { return static_cast<signed long long>(digits[0]); }
    tt_force_inline explicit operator unsigned long () const noexcept { return static_cast<unsigned long>(digits[0]); }
    tt_force_inline explicit operator signed long () const noexcept { return static_cast<signed long>(digits[0]); }
    tt_force_inline explicit operator unsigned int () const noexcept { return static_cast<unsigned int>(digits[0]); }
    tt_force_inline explicit operator signed int () const noexcept { return static_cast<signed int>(digits[0]); }
    tt_force_inline explicit operator unsigned short () const noexcept { return static_cast<unsigned short>(digits[0]); }
    tt_force_inline explicit operator signed short () const noexcept { return static_cast<signed short>(digits[0]); }
    tt_force_inline explicit operator unsigned char () const noexcept { return static_cast<unsigned char>(digits[0]); }
    tt_force_inline explicit operator signed char () const noexcept { return static_cast<signed char>(digits[0]); }
    tt_force_inline explicit operator char () const noexcept { return static_cast<char>(digits[0]); }

    tt_force_inline explicit operator bool () const noexcept {
        for (ssize_t i = 0; i != N; ++i) {
            if (digits[i] != 0) {
                return true;
            }
        }
        return false;
    }

    template<int O>
    tt_force_inline explicit operator bigint<T,O> () const noexcept {
        bigint<T,O> r;

        for (auto i = 0; i < O; i++) {
            r.digits[i] = i < N ? digits[i] : 0;
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
        return fmt::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(digits[1] >> 32),
            static_cast<uint16_t>(digits[1] >> 16),
            static_cast<uint16_t>(digits[1]),
            static_cast<uint16_t>(digits[0] >> 48),
            digits[0] & 0x0000ffff'ffffffffULL
        );
    }

    tt_force_inline digit_type get_bit(unsigned int count) const noexcept {
        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        return (digits[digit_count] >> bit_count) & 1;
    }

    tt_force_inline void set_bit(unsigned int count, digit_type value = 1) noexcept {
        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        digits[digit_count] |= (value << bit_count);
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>, int> = 0>
    tt_force_inline bigint &operator<<=(R rhs) noexcept {
        bigint_shift_left(*this, *this, static_cast<int>(rhs));
        return *this;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>, int> = 0>
    tt_force_inline constexpr bigint &operator>>=(R rhs) noexcept {
        bigint_shift_right(*this, *this, static_cast<int>(rhs));
        return *this;
    }

    tt_force_inline bigint &operator*=(bigint const &rhs) noexcept {
        auto r = bigint<T,N>{0};
        bigint_multiply(r, *this, rhs);
        *this = r;
        return *this;
    }

    tt_force_inline bigint &operator+=(bigint const &rhs) noexcept {
        bigint_add(*this, *this, rhs);
        return *this;
    }

    tt_force_inline bigint &operator-=(bigint const &rhs) noexcept {
        bigint_subtract(*this, *this, rhs);
        return *this;
    }

    tt_force_inline bigint &operator&=(bigint const &rhs) noexcept {
        bigint_and(*this, *this, rhs);
        return *this;
    }

    tt_force_inline bigint &operator|=(bigint const &rhs) noexcept {
        bigint_or(*this, *this, rhs);
        return *this;
    }

    tt_force_inline bigint &operator^=(bigint const &rhs) noexcept {
        bigint_xor(*this, *this, rhs);
        return *this;
    }

    tt_force_inline bigint crc(bigint const &rhs) noexcept {
        return bigint_crc(*this, rhs);
    }

    static tt_force_inline bigint fromBigEndian(uint8_t const *data) noexcept {
        auto r = bigint{};
        for (int i = N - 1; i >= 0; i--) {
            digit_type d = 0;
            for (ssize_t j = 0; j < sizeof(digit_type); j++) {
                d <<= 8;
                d |= *(data++);
            }
            r.digits[i] = d;
        }
        return r;
    }
    static tt_force_inline bigint fromLittleEndian(uint8_t const *data) noexcept {
        auto r = bigint{};
        for (int i = 0; i < N; i++) {
            digit_type d = 0;
            for (ssize_t j = 0; j < sizeof(digit_type); j++) {
                d |= static_cast<digit_type>(*(data++)) << (j*8);
            }
            r.digits[i] = d;
        }
        return r;
    }

    static tt_force_inline bigint fromBigEndian(void const *data) noexcept {
        return fromBigEndian(static_cast<uint8_t const *>(data));
    }

    static tt_force_inline bigint fromLittleEndian(void const *data) noexcept {
        return fromLittleEndian(static_cast<uint8_t const *>(data));
    }

    [[nodiscard]] tt_force_inline friend int bigint_compare(bigint const &lhs, bigint const &rhs) noexcept
    {
        for (int i = N - 1; i >= 0; --i) {
            auto lhs_digit = lhs.digits[i];
            auto rhs_digit = rhs.digits[i];
            if (lhs_digit != rhs_digit) {
                return lhs_digit < rhs_digit ? -1 : 1;
            }
        }
        return 0;
    }

    tt_force_inline friend void bigint_invert(bigint &o, bigint const &rhs) noexcept
    {
        for (auto i = 0; i < N; i++) {
            o.digits[i] = ~rhs.digits[i];
        }
    }

    tt_force_inline friend void bigint_add(bigint &o, bigint const &lhs, bigint const &rhs, T carry=0) noexcept
    {
        for (auto i = 0; i < N; i++) {
            std::tie(o.digits[i], carry) = add_carry(lhs.digits[i], rhs.digits[i], carry);
        }
    }

    tt_force_inline friend void bigint_multiply(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto rhs_index = 0; rhs_index < N; rhs_index++) {
            ttlet rhs_digit = rhs.digits[rhs_index];

            T carry = 0;
            for (auto lhs_index = 0; (lhs_index + rhs_index) < N; lhs_index++) {
                ttlet lhs_digit = lhs.digits[lhs_index];

                T result;
                T accumulator = o.digits[rhs_index + lhs_index];
                std::tie(result, carry) = multiply_carry(lhs_digit, rhs_digit, carry, accumulator);
                o.digits[rhs_index + lhs_index] = result;
            }
        }
    }

    tt_force_inline friend void bigint_div(bigint &quotient, bigint &remainder, bigint const &lhs, bigint const &rhs) noexcept
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

    tt_force_inline friend void bigint_div(bigint &r_quotient, bigint &r_remainder, bigint const &lhs, bigint const &rhs, bigint<T,2*N> const &rhs_reciprocal) noexcept
    {
        auto quotient = bigint<T,3*N>{0};
        bigint_multiply(quotient, lhs, rhs_reciprocal);

        quotient >>= (2*nr_bits);

        auto product = bigint<T,3*N>{0};
        bigint_multiply(product, quotient, rhs);

        tt_assume(product <= lhs);
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
    [[nodiscard]] tt_force_inline friend int bigint_bsr(bigint const &rhs) noexcept
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
    [[nodiscard]] tt_force_inline friend bigint bigint_crc(bigint const &lhs, bigint const &rhs) noexcept
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
    [[nodiscard]] tt_force_inline friend bigint bigint_reciprocal(bigint const &rhs) {
        auto r = bigint<T,N+1>(0);
        r.digits[N] = 1;
        return static_cast<bigint>(r / rhs);
    }


    tt_force_inline friend void bigint_and(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] & rhs.digits[i];
        }
    }

    tt_force_inline friend void bigint_or(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] | rhs.digits[i];
        }
    }

    tt_force_inline friend void bigint_xor(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        for (auto i = 0; i != N; ++i) {
            o.digits[i] = lhs.digits[i] ^ rhs.digits[i];
        }
    }

    tt_force_inline friend void bigint_subtract(bigint &o, bigint const &lhs, bigint const &rhs) noexcept
    {
        bigint rhs_;
        bigint_invert(rhs_, rhs);
        bigint_add(o, lhs, rhs_, T{1});
    }

    tt_force_inline friend void bigint_shift_left(bigint &o, bigint const &lhs, int count) noexcept
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

    tt_force_inline friend void bigint_shift_right(bigint &o, bigint const &lhs, int count) noexcept
    {
        ttlet digit_count = count / bits_per_digit;
        ttlet bit_count = count % bits_per_digit;

        if (&o != &lhs || digit_count > 0) { 
            auto i = 0;
            for (; i != (N - digit_count); ++i) {
                o.digits[i] = lhs.digits[i + digit_count];
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

    [[nodiscard]] tt_force_inline friend bool operator==(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) == 0;
    }

    [[nodiscard]] tt_force_inline friend bool operator<(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) < 0;
    }

    [[nodiscard]] tt_force_inline friend bool operator>(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) > 0;
    }

    [[nodiscard]] tt_force_inline friend bool operator!=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) != 0;
    }

    [[nodiscard]] tt_force_inline friend bool operator>=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) >= 0;
    }

    [[nodiscard]] tt_force_inline friend bool operator<=(bigint const &lhs, bigint const &rhs) noexcept
    {
        return bigint_compare(lhs, rhs) <= 0;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>,int> = 0>
    [[nodiscard]] tt_force_inline friend auto operator<<(bigint const &lhs, R const &rhs) noexcept {
        bigint o;
        bigint_shift_left(o, lhs, static_cast<int>(rhs));
        return o;
    }

    template<typename R, std::enable_if_t<std::is_integral_v<R>,int> = 0>
    [[nodiscard]] tt_force_inline friend auto operator>>(bigint const &lhs, R const &rhs) noexcept {
        bigint o;
        bigint_shift_right(o, lhs, static_cast<int>(rhs));
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator*(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_multiply(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator+(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_add(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator-(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_subtract(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator~(bigint const &rhs) noexcept
    {
        bigint o;
        bigint_invert(o, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator|(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_or(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator&(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_and(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto operator^(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto o = bigint{0};
        bigint_xor(o, lhs, rhs);
        return o;
    }

    [[nodiscard]] tt_force_inline friend auto div(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs);
        return std::pair{ quotient, remainder };
    }

    [[nodiscard]] tt_force_inline friend auto div(bigint const &lhs, bigint const &rhs, bigint<T,2*N> const &rhs_reciprocal) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs, rhs_reciprocal);
        return std::pair{ quotient, remainder };
    }


    [[nodiscard]] tt_force_inline friend auto operator/(bigint const &lhs, bigint const &rhs) noexcept
    {
        auto quotient = bigint{0};
        auto remainder = bigint{0};

        bigint_div(quotient, remainder, lhs, rhs);
        return quotient;
    }

    [[nodiscard]] tt_force_inline friend auto operator%(bigint const &lhs, bigint const &rhs) noexcept
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
