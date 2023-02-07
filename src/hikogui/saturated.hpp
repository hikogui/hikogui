
#include "overflow_int.hpp"
#include <concepts>

#pragma once

namespace hi {
inline namespace v1 {

/** Add with saturation.
 *
 * @param lhs The left hand side of the add operation.
 * @param rhs The right hand side of the add operation.
 * @return The sum, or satured to minimum and maximum of the integer types.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_add(T lhs, T rhs) noexcept
{
    using UT = std::make_unsigned_t<T>;
    constexpr auto T_BIT = sizeof(T) * CHAR_BIT;
    constexpr auto UT_MAX = sign_cast<UT>(std::numeric_limits<T>::max());

    auto r = T{};
    if (overflow_add(lhs, rhs, &r)) {
        r = sign_cast<T>((sign_cast<UT>(lhs) >> (T_BIT - 1)) + UT_MAX);
    }

    return r;
}

/** Subtract with saturation.
 *
 * @param lhs The left hand side of the subtract operation.
 * @param rhs The right hand side of the subtract operation.
 * @return The difference, or satured to minimum and maximum of the integer types.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_sub(T lhs, T rhs) noexcept
{
    using UT = std::make_unsigned_t<T>;
    constexpr auto T_BIT = sizeof(T) * CHAR_BIT;
    constexpr auto UT_MAX = sign_cast<UT>(std::numeric_limits<T>::max());

    auto r = T{};
    if (overflow_sub(lhs, rhs, &r)) {
        r = sign_cast<T>((sign_cast<UT>(lhs) >> (T_BIT - 1)) + UT_MAX);
    }

    return r;
}

/** Multiply with saturation.
 *
 * @param lhs The left hand side of the multiplication operation.
 * @param rhs The right hand side of the multiplication operation.
 * @return The product, or satured to minimum and maximum of the integer types.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_mul(T lhs, T rhs) noexcept
{
    using UT = std::make_unsigned_t<T>;
    constexpr auto T_BIT = sizeof(T) * CHAR_BIT;
    constexpr auto UT_MAX = sign_cast<UT>(std::numeric_limits<T>::max());

    auto r = T{};
    if (overflow_mul(lhs, rhs, &r)) {
        r = sign_cast<T>((sign_cast<UT>(lhs ^ rhs) >> (T_BIT - 1)) + UT_MAX);
    }

    return r;
}

/** Divide with saturation.
 *
 * @note divide by zero results in a minimum if lhs is negative, or maximum is lhs is positive.
 *       This mirrors floating point divide by zero.
 * @param lhs The left hand side of the division operation.
 * @param rhs The right hand side of the division operation.
 * @return The division result, or satured to minimum and maximum of the integer types.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_div(T lhs, T rhs) noexcept
{
    using UT = std::make_unsigned_t<T>;
    constexpr auto T_BIT = sizeof(T) * CHAR_BIT;
    constexpr auto UT_MAX = sign_cast<UT>(std::numeric_limits<T>::max());

    auto r = T{};
    if (overflow_div(lhs, rhs, &r)) {
        r = sign_cast<T>((sign_cast<UT>(lhs ^ rhs) >> (T_BIT - 1)) + UT_MAX);
    }

    return r;
}

/** Modulo with saturation.
 *
 * r = lhs - rhs * truncate(lhs / rhs).
 *
 * @param lhs The left hand side of the division operation.
 * @param rhs The right hand side of the division operation.
 * @return The division result, or satured to minimum and maximum of the integer types.
 * @throw When modulo by zero.
 */
template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_mod(T lhs, T rhs)
{
}

template<std::signed_integral T>
[[nodiscard]] constexpr T saturate_abs(T rhs) noexcept
{
    using UT = std::make_unsigned_t<T>;
   
}

template<std::signed_integral T>
struct saturated_int {
    using value_type = T;
    using unsigned_type = std::make_unsigned_t<value_type>;

    constexpr static size_t value_bit = sizeof(value_type) * CHAR_BIT;
    constexpr static value_type int_max = std::numeric_limits<value_type>::max();
    constexpr static value_type int_min = std::numeric_limits<value_type>::min();
    constexpr static unsigned_type unsigned_int_max = static_cast<unsigned_type>(int_max);

    value_type v = 0;

    constexpr saturated_int() noexcept = default;
    constexpr saturated_int(saturated_int const &) noexcept = default;
    constexpr saturated_int(saturated_int &&) noexcept = default;
    constexpr saturated_int &operator=(saturated_int const &) noexcept = default;
    constexpr saturated_int &operator=(saturated_int &&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(saturated_int const &, saturated_int const &) noexcept = default;
    [[nodiscard]] constexpr friend auto operator<=>(saturated_int const &, saturated_int const &) noexcept = default;



    [[nodiscard]] constexpr friend saturated_int operator+(saturated_int const &lhs, saturated_int const &rhs) noexcept
    {
        return {saturate_add(lhs.v, rhs.v)};
    }

    [[nodiscard]] constexpr friend saturated_int operator-(saturated_int const &lhs, saturated_int const &rhs) noexcept
    {
        return {saturate_sub(lhs.v, rhs.v)};
    }

    [[nodiscard]] constexpr friend saturated_int operator*(saturated_int const &lhs, saturated_int const &rhs) noexcept
    {
        return {saturate_mul(lhs.v, rhs.v)};
    }

    [[nodiscard]] constexpr friend saturated_int operator/(saturated_int const &lhs, saturated_int const &rhs) noexcept
    {
        return {saturate_div{lhs.v, rhs.v};
    }

};


}}

