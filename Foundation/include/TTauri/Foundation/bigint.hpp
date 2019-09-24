// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Required/strings.hpp"
#include "TTauri/Foundation/math.hpp"
#include <type_traits>

namespace TTauri {

/*!
 *
 * We do not include any default, copy, move constructor or operators and destructor.
 * for performance reasons.
 */
template<typename T, int N>
struct bigint {
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

    constexpr explicit bigint(digit_type value) noexcept {
        digits[0] = value;
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

    constexpr digit_type get_bit(unsigned int count) const noexcept {
        let digit_count = to_int(count / bits_per_digit);
        let bit_count = count % bits_per_digit;

        return (digits[digit_count] >> bit_count) & 1;
    }

    constexpr void set_bit(unsigned int count, digit_type value = 1) noexcept {
        let digit_count = to_int(count / bits_per_digit);
        let bit_count = count % bits_per_digit;

        digits[digit_count] |= (value << bit_count);
    }

    constexpr bigint &operator<<=(unsigned int count) noexcept {
        let digit_count = to_int(count / bits_per_digit);
        let bit_count = count % bits_per_digit;

        if (digit_count > 0) {
            for (auto i = nr_digits - 1; i >= digit_count; i--) {
                digits[i] = digits[i - digit_count];
            }
            for (auto i = digit_count - 1; i >= 0; i--) {
                digits[i] = 0;
            }
        }
        if (bit_count > 0) {
            digit_type carry = 0;
            for (auto i = 0; i < nr_digits; i++) {
                std::tie(digits[i], carry) = shift_left_carry(digits[i], bit_count, carry);
            }
        }
        return *this;
    }

    constexpr bigint &operator>>=(unsigned int count) noexcept {
        let digit_count = to_int(count / bits_per_digit);
        let bit_count = count % bits_per_digit;

        if (digit_count > 0) {
            auto i = 0;
            for (; i < (nr_digits - digit_count); i++) {
                digits[i] = digits[i + digit_count];
            }
            for (; i < nr_digits; i++) {
                digits[i] = 0;
            }
        }
        if (bit_count > 0) {
            digit_type carry = 0;
            for (auto i = nr_digits - 1; i >= 0; i--) {
                std::tie(digits[i], carry) = shift_right_carry(digits[i], bit_count, carry);
            }
        }

        return *this;
    }

    constexpr bigint &operator*=(digit_type rhs) noexcept {
        digit_type carry = 0;
        for (auto i = 0; i < nr_digits; i++) {
            std::tie(digits[i], carry) = multiply_carry(digits[i], rhs, carry);
        }
        return *this;
    }

    constexpr bigint &operator+=(digit_type rhs) noexcept {
        digit_type carry = 0;
        std::tie(digits[0], carry) = add_carry(digits[0], rhs, digit_type{0});
        for (auto i = 1; i < nr_digits; i++) {
            std::tie(digits[i], carry) = add_carry(digits[i], digit_type{0}, carry);
        }
        return *this;
    }

    constexpr bigint &operator-=(digit_type rhs) noexcept {
        digit_type borrow = 0;
        std::tie(digits[0], borrow) = subtract_borrow(digits[0], rhs, digit_type{0});
        for (auto i = 1; i < nr_digits; i++) {
            std::tie(digits[i], borrow) = subtract_borrow(digits[i], digit_type{0}, borrow);
        }
        return *this;
    }

    constexpr bigint &operator&=(digit_type rhs) noexcept {
        digits[0] &= rhs;
        for (int i = 1; i < nr_digits; i++) {
            digits[i] = 0;
        }
        return *this;
    }

    constexpr bigint &operator|=(digit_type rhs) noexcept {
        digits[0] |= rhs;
        return *this;
    }

    constexpr bigint &operator^=(digit_type rhs) noexcept {
        digits[0] ^= rhs;
        return *this;
    }

    constexpr bigint &operator^=(bigint const &rhs) noexcept {
        for (auto i = 0; i < nr_digits; i++) {
            digits[i] ^= rhs.digits[i];
        }
        return *this;
    }

    constexpr bigint &operator|=(bigint const &rhs) noexcept {
        for (auto i = 0; i < nr_digits; i++) {
            digits[i] |= rhs.digits[i];
        }
        return *this;
    }
};

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator==(bigint<T,N> const &lhs, U const rhs) noexcept
{
    for (auto i = N-1; i >= 1; i--) {
        if (lhs.digits[i] != 0) {
            return false;
        }
    }
    return lhs.digits[0] == static_cast<T>(rhs);
}


template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator<(bigint<T,N> const &lhs, U const rhs) noexcept
{
    for (auto i = N-1; i >= 1; i--) {
        if (lhs.digits[i] != 0) {
            return false;
        }
    }
    return lhs.digits[0] < static_cast<T>(rhs);
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator>(bigint<T,N> const &lhs, U rhs) noexcept {
    for (auto i = N-1; i >= 1; i--) {
        if (lhs.digits[i] != 0) {
            return true;
        }
    }
    return lhs.digits[0] > static_cast<T>(rhs);
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator!=(bigint<T,N> const &lhs, U rhs) noexcept {
    return !(lhs == rhs);
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator>=(bigint<T,N> const &lhs, U rhs) noexcept {
    return !(lhs < rhs);
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bool operator<=(bigint<T,N> const &lhs, U rhs) noexcept {
    return !(lhs > rhs);
}



template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator<<(bigint<T,N> const &lhs, U count) noexcept {
    let digit_count = to_int(count) / bigint<T,N>::bits_per_digit;
    let bit_count = to_int(count) % bigint<T,N>::bits_per_digit;

    bigint<T,N> r;

    // Always copy the digits into r.
    for (auto i = N - 1; i >= digit_count; i--) {
        r.digits[i] = lhs.digits[i - digit_count];
    }
    for (auto i = digit_count - 1; i >= 0; i--) {
        r.digits[i] = 0;
    }

    if (bit_count > 0) {
        T carry = 0;
        for (auto i = 0; i < N; i++) {
            std::tie(r.digits[i], carry) = shift_left_carry(r.digits[i], bit_count, carry);
        }
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator*(bigint<T,N> const &lhs, U rhs) noexcept {
    bigint<T,N> r;

    T carry = 0;
    for (auto i = 0; i < nr_digits; i++) {
        std::tie(r.digits[i], carry) = multiply_carry(lhs.digits[i], static_cast<T>(rhs), carry);
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator+(bigint<T,N> const &lhs, U rhs) noexcept {
    bigint<T,N> r;

    T carry = 0;
    for (auto i = 0; i < nr_digits; i++) {
        std::tie(r.digits[i], carry) = add_carry(lhs.digits[i], static_cast<T>(rhs), carry);
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator-(bigint<T,N> const &lhs, U rhs) noexcept {
    bigint<T,N> r;

    T borrow = 0;
    for (auto i = 0; i < N; i++) {
        std::tie(r.digits[i], borrow) = subtract_borrow(lhs.digits[i], static_cast<T>(rhs), borrow);
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr U operator&(bigint<T,N> const &lhs, U rhs) noexcept {
    return static_cast<U>(lhs.digits[0] & static_cast<T>(rhs));
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator|(bigint<T,N> const &lhs, U rhs) noexcept {
    bigint<T,N> r;

    r.digits[0] = lhs.digits[0] | static_cast<T>(rhs);
    for (int i = 1; i < N; i++) {
        r.digits[i] = 0;
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr bigint<T,N> operator^(bigint<T,N> const &lhs, U rhs) noexcept {
    bigint<T,N> r;

    r.digits[0] = lhs.digits[0] ^ static_cast<T>(rhs);
    for (int i = 1; i < N; i++) {
        r.digits[i] = 0;
    }
    return r;
}

template<typename T, int N, typename U, std::enable_if_t<std::is_integral_v<U>,int> = 0>
constexpr std::pair<bigint<T,N>, bigint<T,N>> div(bigint<T,N> const &lhs, U rhs) noexcept
{
    auto quotient = bigint<T,N>{0};
    auto remainder = bigint<T,N>{0};

    for (auto i = bigint<T,N>::nr_bits - 1; i >= 0; i--) {
        remainder <<= 1;
        remainder |= lhs.get_bit(i);
        if (remainder >= static_cast<T>(rhs)) {
            remainder -= static_cast<T>(rhs);
            quotient.set_bit(i);
        }
    }

    return { quotient, remainder };
}


template<typename T, int N>
constexpr bool operator==(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept
{
    for (auto i = N-1; i >= 0; i--) {
        if (lhs.digits[i] != rhs.digits[i]) {
            return false;
        }
    }
    return true;
}

template<typename T, int N>
constexpr bool operator<(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept
{
    for (auto i = N-1; i >= 0; i--) {
        if (lhs.digits[i] >= rhs.digits[i]) {
            return false;
        }
    }
    return true;
}

template<typename T, int N> constexpr bool operator>(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept { return rhs < lhs; }
template<typename T, int N> constexpr bool operator!=(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept { return !(lhs == rhs); }
template<typename T, int N> constexpr bool operator>=(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept { return !(lhs < rhs); }
template<typename T, int N> constexpr bool operator<=(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept { return !(lhs > rhs); }


template<typename T, int N, int O>
constexpr auto operator|(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept {
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;

    for (auto i = 0; i < nr_digits; i++) {
        auto lhs_digit = i < N ? lhs.digits[i] : static_cast<T>(0);
        auto rhs_digit = i < O ? rhs.digits[i] : static_cast<T>(0);
        r.digits[i] = lhs_digit | rhs_digit;
    }
    return r;
}

template<typename T, int N, int O>
constexpr auto operator^(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept {
    constexpr int nr_digits = std::max(N, O);
    bigint<T,nr_digits> r;

    for (auto i = 0; i < nr_digits; i++) {
        auto lhs_digit = i < N ? lhs.digits[i] : static_cast<T>(0);
        auto rhs_digit = i < O ? rhs.digits[i] : static_cast<T>(0);
        r.digits[i] = lhs_digit ^ rhs_digit;
    }
    return r;
}

template<typename T, int N, int O>
constexpr auto operator&(bigint<T,N> const &lhs, bigint<T,O> const &rhs) noexcept {
    constexpr int nr_digits = std::min(N, O);
    bigint<T,nr_digits> r;

    for (auto i = 0; i < nr_digits; i++) {
        r.digits[i] = lhs.digits[i] ^ rhs.digits[i];
    }
    return r;
}

template<typename T, int N>
constexpr std::pair<bigint<T,N>, bigint<T,N>> div(bigint<T,N> const &lhs, bigint<T,N> const &rhs) noexcept
{
    bigint<T,N> quotient = 0;
    bigint<T,N> remainder = 0;

    for (i = bigint<T,N>::nr_bits - 1; i >= 0; i--) {
        remainder <<= 1;
        remainder |= lhs.get_bit(i);
        if (remainder >= rhs) {
            remainder -= rhs;
            quotient.set_bit(i);
        }
    }

    return { quotient, remainder };
}

using uint128_t = bigint<uint64_t,2>;

}
