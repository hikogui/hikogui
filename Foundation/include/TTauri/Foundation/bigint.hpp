// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/math.hpp"
#include <fmt/format.h>
#include <type_traits>
#include <ostream>

namespace TTauri {

template<typename T, int N, bool SIGNED=false>
struct bigint;

template<typename T, int N, int O>
constexpr bigint<T,N> bigint_reciprocal(bigint<T,O> const &divider);

template<typename T, int N, typename U>
constexpr bigint<T,N> bigint_reciprocal(U const &divider);

/*! High performance big integer implementation.
 * The bigint is a fixed width integer which will allow the compiler
 * to make aggressive optimizations, unrolling most loops and easy inlining.
 */
template<typename T, int N, bool SIGNED>
struct bigint {
    static_assert(N >= 0, "bigint must have zero or more digits.");
    static_assert(!SIGNED, "bigint has not implemented signed integers yet.");
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>, "bigint's digit must be unsigned integers.");

    using digit_type = T;
    static constexpr int nr_digits = N;
    static constexpr int bits_per_digit = sizeof(digit_type) * 8;
    static constexpr int nr_bits = nr_digits * bits_per_digit;

    /*! Digits, in little endian order.
    */
    std::array<digit_type,nr_digits> digits;

    constexpr bigint() noexcept = default;
    ~bigint() = default;
    constexpr bigint(bigint const &) noexcept = default;
    constexpr bigint &operator=(bigint const &) noexcept = default;
    constexpr bigint(bigint &&) noexcept = default;
    constexpr bigint &operator=(bigint &&) noexcept = default;

    template<typename U, std::enable_if_t<std::is_integral_v<U>, int> = 0>
    constexpr explicit bigint(U value) noexcept {
        digits[0] = static_cast<digit_type>(value);
        for (auto i = 1; i < nr_digits; i++) {
            digits[i] = 0;
        }
    }

    constexpr bigint &operator=(digit_type value) noexcept {
        digits[0] = value;
        for (auto i = 1; i < nr_digits; i++) {
            digits[i] = 0;
        }
        return *this;
    }

    constexpr explicit bigint(std::string_view str, int base=10) noexcept :
        bigint(0)
    {
        for (auto i = 0; i < str.size(); i++) {
            auto nibble = char_to_nibble(str[i]);
            (*this) *= base;
            (*this) += nibble;
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
    constexpr explicit operator char () const noexcept { return static_cast<char>(digits[0]); }

    template<int O>
    constexpr explicit operator bigint<T,O> () const noexcept {
        bigint<T,O> r;

        for (auto i = 0; i < O; i++) {
            r.digits[i] = i < N ? digits[i] : 0;
        }
        return r;
    }

    std::string string() const noexcept {
        static auto oneOver10 = bigint_reciprocal<T,N*2>(10);

        auto tmp = *this;

        std::string r;

        if (tmp == 0) {
            r = "0";
        } else {
            while (tmp > 0) {
                digit_type remainder;
                std::tie(tmp, remainder) = div(tmp, 10, oneOver10);
                r += (static_cast<char>(remainder) + '0');
            }
        }

        std::reverse(r.begin(), r.end());
        return r;
    }

    std::string UUIDString() const noexcept {
        static_assert(std::is_same_v<T,uint64_t> && N == 2 && !SIGNED, "UUIDString should only be called on a uuid compatible type");
        return fmt::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}",
            static_cast<uint32_t>(digits[1] >> 32),
            static_cast<uint16_t>(digits[1] >> 16),
            static_cast<uint16_t>(digits[1]),
            static_cast<uint16_t>(digits[0] >> 48),
            digits[0] & 0x0000ffff'ffffffffULL
        );
    }

    constexpr digit_type get_bit(unsigned int count) const noexcept {
        let digit_count = count / bits_per_digit;
        let bit_count = count % bits_per_digit;

        return (digits[digit_count] >> bit_count) & 1;
    }

    constexpr void set_bit(unsigned int count, digit_type value = 1) noexcept {
        let digit_count = count / bits_per_digit;
        let bit_count = count % bits_per_digit;

        digits[digit_count] |= (value << bit_count);
    }

    constexpr bigint &operator<<=(unsigned int count) noexcept {
        bigint_shift_left(*this, *this, count);
        return *this;
    }

    constexpr bigint &operator>>=(unsigned int count) noexcept {
        bigint_shift_right(*this, *this, count);
        return *this;
    }

    template<int O>
    constexpr bigint &operator*=(bigint<digit_type,O> const &rhs) noexcept {
        auto r = bigint<T,N>{0};
        bigint_multiply(r, *this, rhs);
        *this = r;
        return *this;
    }

    template<typename U>
    constexpr bigint &operator*=(U const &rhs) noexcept {
        return (*this) *= bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint &operator+=(bigint<digit_type,O> const &rhs) noexcept {
        bigint_add(*this, *this, rhs);
        return *this;
    }

    template<typename U>
    constexpr bigint &operator+=(U const &rhs) noexcept {
        return (*this) += bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint &operator-=(bigint<digit_type,O> const &rhs) noexcept {
        bigint_subtract(*this, *this, rhs);
        return *this;
    }

    template<typename U>
    constexpr bigint &operator-=(U const &rhs) noexcept {
        return (*this) -= bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint &operator&=(bigint<digit_type,O> const &rhs) noexcept {
        bigint_and(*this, *this, rhs);
        return *this;
    }

    template<typename U>
    constexpr bigint &operator&=(U const &rhs) noexcept {
        return (*this) &= bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint &operator|=(bigint<digit_type,O> const &rhs) noexcept {
        bigint_or(*this, *this, rhs);
        return *this;
    }

    template<typename U>
    constexpr bigint &operator|=(U const &rhs) noexcept {
        return (*this) |= bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint &operator^=(bigint<digit_type,O> const &rhs) noexcept {
        bigint_xor(*this, *this, rhs);
        return *this;
    }

    template<typename U>
    constexpr bigint &operator^=(U const &rhs) noexcept {
        return (*this) ^= bigint<digit_type,1>{rhs};
    }

    template<int O>
    constexpr bigint<digit_type,O> crc(bigint<digit_type,O> const &rhs) noexcept {
        return bigint_crc(*this, rhs);
    }

    template<typename U>
    constexpr U crc(U const &rhs) noexcept {
        return static_cast<U>(crc(bigint<digit_type,1>{rhs}));
    }

    static constexpr bigint fromBigEndian(uint8_t const *data) noexcept {
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
    static constexpr bigint fromLittleEndian(uint8_t const *data) noexcept {
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

    static constexpr bigint fromBigEndian(void const *data) noexcept {
        return fromBigEndian(static_cast<uint8_t const *>(data));
    }

    static constexpr bigint fromLittleEndian(void const *data) noexcept {
        return fromLittleEndian(static_cast<uint8_t const *>(data));
    }
};

template<typename T, int N, int O>
constexpr int bigint_compare(bigint<T,N> const &lhs, bigint<T,O> const &rhs, T carry=0) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    for (auto i = nr_digits - 1; i >= 0; i--) {
        let lhs_digit = i < N ? lhs.digits[i] : T{0};
        let rhs_digit = i < O ? rhs.digits[i] : T{0};

        if (lhs_digit < rhs_digit) {
            return -1;
        } else if (lhs_digit > rhs_digit) {
            return 1;
        }
    }
    return 0;
}

template<typename T, int R, int N>
constexpr void bigint_invert(bigint<T,R> &r, bigint<T,N> const &rhs) noexcept
{
    for (auto i = 0; i < R; i++) {
        r.digits[i] = i < N ? ~rhs.digits[i] : ~T{0};
    }
}

template<typename T, int R, int N, int O>
constexpr void bigint_add(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs, T carry=0) noexcept
{
    for (auto i = 0; i < R; i++) {
        let lhs_digit = i < N ? lhs.digits[i] : T{0};
        let rhs_digit = i < O ? rhs.digits[i] : T{0};
        std::tie(r.digits[i], carry) = add_carry(lhs_digit, rhs_digit, carry);
    }
}

template<typename T, int R, int N, int O>
constexpr void bigint_multiply(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    for (auto rhs_index = 0; rhs_index < O; rhs_index++) {
        let rhs_digit = rhs.digits[rhs_index];

        T carry = 0;
        for (auto lhs_index = 0; lhs_index < N; lhs_index++) {
            let lhs_digit = lhs.digits[lhs_index];

            T result;
            T accumulator = (rhs_index + lhs_index) < R ? r.digits[rhs_index + lhs_index] : 0;
            std::tie(result, carry) = multiply_carry(lhs_digit, rhs_digit, carry, accumulator);
            if ((rhs_index + lhs_index) < R) {
                r.digits[rhs_index + lhs_index] = result;
            }
        }

        // Save the overflow in the digit beyond the current most significant byte.
        if (rhs_index + N < R) {
            r.digits[rhs_index + N] = carry;
        }
    }
}

template<typename T, int R, int S, int N, int O>
constexpr void bigint_div(bigint<T,R> &quotient, bigint<T,S> &remainder, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    for (auto i = bigint<T,N>::nr_bits - 1; i >= 0; i--) {
        remainder <<= 1;
        remainder |= lhs.get_bit(i);
        if (remainder >= rhs) {
            remainder -= rhs;
            quotient.set_bit(i);
        }
    }
}

template<typename T, int R, int S, int N, int O, int P>
constexpr void bigint_div(bigint<T,R> &r_quotient, bigint<T,S> &r_remainder, bigint<T,N> const &lhs, bigint<T,O> const &rhs, bigint<T,P> const &rhs_reciprocal) noexcept
{
    auto quotient = bigint<T,N+P>{0};
    bigint_multiply(quotient, lhs, rhs_reciprocal);

    quotient >>= P*sizeof(T)*8;

    auto product = bigint<T,N+O>{0};
    bigint_multiply(product, quotient, rhs);

    optional_assert(product <= lhs);
    auto remainder = lhs - product;

    int retry = 0;
    while (remainder >= rhs) {
        if (retry++ > 3) {
            required_assert(false);
            bigint_div(r_quotient, r_remainder, lhs, rhs);
        }

        remainder -= rhs;
        quotient += 1;
    }
    r_quotient = static_cast<bigint<T,R>>(quotient);
    r_remainder = static_cast<bigint<T,S>>(remainder);
}

/*! Bit scan reverse.
 * \return index of leading one, or -1 when rhs is zero.
 */
template<typename T, int N>
constexpr int bigint_bsr(bigint<T,N> const &rhs) noexcept
{
    for (auto i = N - 1; i >= 0; i--) {
        auto tmp = bsr(rhs.digits[i]);
        if (tmp >= 0) {
            return (i * bigint<T,N>::bits_per_digit) + tmp;
        }
    }
    return -1;
}

/*! Calculate the remainder of a CRC check.
 * \param r Return value, the remainder.
 * \param lhs The number to check.
 * \param rhs Polynomial.
 */
template<typename T, int N, int O>
constexpr bigint<T,O> bigint_crc(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    let polynomialOrder = bigint_bsr(rhs);
    required_assert(polynomialOrder >= 0);

    auto tmp = static_cast<bigint<T,O+N>>(lhs) << polynomialOrder;
    auto rhs_ = static_cast<bigint<T,O+N>>(rhs);

    auto tmp_highest_bit = bigint_bsr(tmp);
    while (tmp_highest_bit >= polynomialOrder) {
        let divident = rhs_ << (tmp_highest_bit - polynomialOrder);

        tmp ^= divident;
        tmp_highest_bit = bigint_bsr(tmp);
    }

    return static_cast<bigint<T,O>>(tmp);
}

/*! Calculate the reciprocal at a certain precision.
 *
 * N should be two times the size of the eventual numerator.
 *
 * \param divider The divider of 1.
 * \return (1 << (K*sizeof(T)*8)) / divider
 */
template<typename T, int N, int O>
constexpr bigint<T,N> bigint_reciprocal(bigint<T,O> const &divider) {
    auto r = bigint<T,N+1>(0);
    r.digits[N] = 1;
    return static_cast<bigint<T,N>>(r / divider);
}

template<typename T, int N, typename U>
constexpr bigint<T,N> bigint_reciprocal(U const &divider) {
    return bigint_reciprocal<T,N>(bigint<T,1>{divider});
}

template<typename T, int R, int N, int O>
constexpr void bigint_and(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    for (auto i = 0; i < R; i++) {
        let lhs_digit = i < N ? lhs.digits[i] : T{0};
        let rhs_digit = i < O ? rhs.digits[i] : T{0};
        r.digits[i] = lhs_digit & rhs_digit;
    }
}

template<typename T, int R, int N, int O>
constexpr void bigint_or(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    for (auto i = 0; i < R; i++) {
        let lhs_digit = i < N ? lhs.digits[i] : T{0};
        let rhs_digit = i < O ? rhs.digits[i] : T{0};
        r.digits[i] = lhs_digit | rhs_digit;
    }
}

template<typename T, int R, int N, int O>
constexpr void bigint_xor(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    for (auto i = 0; i < R; i++) {
        let lhs_digit = i < N ? lhs.digits[i] : T{0};
        let rhs_digit = i < O ? rhs.digits[i] : T{0};
        r.digits[i] = lhs_digit ^ rhs_digit;
    }
}

template<typename T, int R, int N, int O>
constexpr void bigint_subtract(bigint<T,R> &r, bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    bigint<T,R> rhs_;
    bigint_invert(rhs_, rhs);
    bigint_add(r, lhs, rhs_, T{1});
}

template<typename T, int N>
constexpr void bigint_shift_left(bigint<T,N> &r, bigint<T,N> const &lhs, int count) noexcept
{
    let digit_count = count / bigint<T,N>::bits_per_digit;
    let bit_count = count % bigint<T,N>::bits_per_digit;

    if (&r != &lhs || digit_count > 0) { 
        for (auto i = N - 1; i >= digit_count; i--) {
            r.digits[i] = lhs.digits[i - digit_count];
        }
        for (auto i = digit_count - 1; i >= 0; i--) {
            r.digits[i] = 0;
        }
    }

    if (bit_count > 0) {
        T carry = 0;
        for (auto i = 0; i < N; i++) {
            auto tmp = shift_left_carry(r.digits[i], bit_count, carry);
            r.digits[i] = tmp.first;
            carry = tmp.second;
            //std::tie(r.digits[i], carry) = ;
        }
    }
}

template<typename T, int N>
constexpr void bigint_shift_right(bigint<T,N> &r, bigint<T,N> const &lhs, int count) noexcept
{
    let digit_count = count / bigint<T,N>::bits_per_digit;
    let bit_count = count % bigint<T,N>::bits_per_digit;

    if (&r != &lhs || digit_count > 0) { 
        auto i = 0;
        for (; i < (N - digit_count); i++) {
            r.digits[i] = lhs.digits[i + digit_count];
        }
        for (; i < N; i++) {
            r.digits[i] = 0;
        }
    }

    if (bit_count > 0) {
        T carry = 0;
        for (auto i = N - 1; i >= 0; i--) {
            std::tie(r.digits[i], carry) = shift_right_carry(r.digits[i], bit_count, carry);
        }
    }
}

template<typename T, int N, int O>
constexpr bool operator==(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) == 0;
}

template<typename T, int N, int O>
constexpr bool operator<(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) < 0;
}

template<typename T, int N, int O>
constexpr bool operator>(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) > 0;
}

template<typename T, int N, int O>
constexpr bool operator!=(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) != 0;
}

template<typename T, int N, int O>
constexpr bool operator>=(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) >= 0;
}

template<typename T, int N, int O>
constexpr bool operator<=(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    return bigint_compare(lhs, rhs) <= 0;
}

template<typename T, typename U, int N>
constexpr bool operator==(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs == bigint<T,1>{rhs}; }
template<typename T, typename U, int N>
constexpr bool operator!=(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs != bigint<T,1>{rhs}; }
template<typename T, typename U, int N>
constexpr bool operator<(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs < bigint<T,1>{rhs}; }
template<typename T, typename U, int N>
constexpr bool operator>(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs > bigint<T,1>{rhs}; }
template<typename T, typename U, int N>
constexpr bool operator<=(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs <= bigint<T,1>{rhs}; }
template<typename T, typename U, int N>
constexpr bool operator>=(bigint<T,N> const &lhs, U const &rhs) noexcept { return lhs >= bigint<T,1>{rhs}; }


template<typename T, int N, typename U>
constexpr bigint<T,N> operator<<(bigint<T,N> const &lhs, U count) noexcept {
    bigint<T,N> r;
    bigint_shift_left(r, lhs, static_cast<int>(count));
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator>>(bigint<T,N> const &lhs, U count) noexcept {
    bigint<T,N> r;
    bigint_shift_right(r, lhs, static_cast<int>(count));
    return r;
}


template<typename T, int N, int O>
constexpr auto operator*(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    auto r = bigint<T,nr_digits>{0};
    bigint_multiply(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator*(bigint<T,N> const &lhs, U rhs) noexcept {
    return lhs * bigint<T,1>{rhs};
}

template<typename T, int N, int O>
constexpr auto operator+(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;
    bigint_add(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator+(bigint<T,N> const &lhs, U rhs) noexcept {
    return lhs + bigint<T,1>{rhs};
}

template<typename T, int N, int O>
constexpr auto operator-(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;
    bigint_subtract(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator-(bigint<T,N> const &lhs, U rhs) noexcept {
    return lhs - bigint<T,1>{rhs};
}

template<typename T, int N>
constexpr auto operator~(bigint<T,N> const &rhs) noexcept
{
    bigint<T,N> r;
    bigint_invert(r, rhs);
    return r;
}

template<typename T, int N, int O>
constexpr auto operator|(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;
    bigint_or(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator|(bigint<T,N> const &lhs, U rhs) noexcept {
    return lhs | bigint<T,1>{rhs};
}

template<typename T, int N, int O>
constexpr auto operator&(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::min(N, O);
    bigint<T,nr_digits> r;
    bigint_and(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr U operator&(bigint<T,N> const &lhs, U rhs) noexcept {
    return static_cast<U>(lhs & bigint<T,1>{rhs});
}

template<typename T, int N, int O>
constexpr auto operator^(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;
    bigint_xor(r, lhs, rhs);
    return r;
}

template<typename T, int N, typename U>
constexpr bigint<T,N> operator^(bigint<T,N> const &lhs, U rhs) noexcept {
    return lhs ^ bigint<T,1>{rhs};
}

template<typename T, int N, int O>
constexpr std::pair<bigint<T,N>, bigint<T,O>> div(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    bigint<T,N> quotient = 0;
    bigint<T,O> remainder = 0;

    bigint_div(quotient, remainder, lhs, rhs);
    return { quotient, remainder };
}

template<typename T, int N, typename U>
constexpr std::pair<bigint<T,N>, U> div(bigint<T,N> const &lhs, U rhs) noexcept
{
    auto quotient = bigint<T,N>{0};
    auto remainder = bigint<T,1>{0};

    bigint_div(quotient, remainder, lhs, bigint<T,1>{rhs});
    return { quotient, static_cast<U>(remainder) };
}

template<typename T, int N, int O, int P>
constexpr std::pair<bigint<T,N>, bigint<T,O>> div(bigint<T,N> const &lhs, bigint<T,O> const &rhs, bigint<T,P> const &rhs_reciprocal) noexcept
{
    bigint<T,N> quotient = 0;
    bigint<T,O> remainder = 0;

    bigint_div(quotient, remainder, lhs, rhs, rhs_reciprocal);
    return { quotient, remainder };
}

template<typename T, int N, typename U, int O>
constexpr std::pair<bigint<T,N>, U> div(bigint<T,N> const &lhs, U rhs, bigint<T,O> const &rhs_reciprocal) noexcept
{
    auto quotient = bigint<T,N>{0};
    auto remainder = bigint<T,1>{0};

    bigint_div(quotient, remainder, lhs, bigint<T,1>{rhs}, rhs_reciprocal);
    return { quotient, static_cast<U>(remainder) };
}

template<typename T, int N, int O>
constexpr bigint<T,N> operator/(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept
{
    auto quotient = bigint<T,N>{0};
    auto remainder = bigint<T,O>{0};

    bigint_div(quotient, remainder, lhs, rhs);
    return quotient;
}

//template<typename T, typename U, int N, std::enable_if_t<std::is_integral_v<U>, int> = 0>
template<typename T, typename U, int N>
constexpr bigint<T,N> operator/(bigint<T,N> const &lhs, U const &rhs) noexcept
{
    return lhs / bigint<T,1>{rhs};
}

template<typename T, int N, int O>
constexpr bigint<T,O> operator%(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept
{
    bigint<T,N> quotient = 0;
    bigint<T,O> remainder = 0;

    bigint_div(quotient, remainder, lhs, rhs);
    return remainder;
}

template<typename T, typename U, int N>
constexpr U operator%(bigint<T,N> const &lhs, U const &rhs) noexcept
{
    return static_cast<U>(lhs % bigint<T,1>{rhs});
}

template<typename T, int N>
inline std::ostream &operator<<(std::ostream &lhs, bigint<T,N> const &rhs) {
    lhs << rhs.string();
    return lhs;
}

using ubig128 = bigint<uint64_t,2>;
using uuid = bigint<uint64_t,2>;

}
