// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file utility/compare.ixx Safe numeric comparison between different types.
 */

#pragma once

#include "type_traits.hpp"
#include "../macros.hpp"
#include "concepts.hpp"
#include <concepts>
#include <compare>

hi_export_module(hikogui.utility.compare);

hi_export namespace hi { inline namespace v1 {

/** A functor to safely compare two arithmetic values.
 */
template<typename Lhs, typename Rhs>
struct three_way_comparison;

template<std::unsigned_integral Lhs, std::unsigned_integral Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
    {
        return lhs <=> rhs;
    }
};

template<std::signed_integral Lhs, std::signed_integral Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
    {
        return lhs <=> rhs;
    }
};

template<std::floating_point Lhs, std::floating_point Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::partial_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
    {
        return lhs <=> rhs;
    }
};

template<std::signed_integral Lhs, std::unsigned_integral Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
        requires(sizeof(Rhs) < sizeof(long long))
    {
        return lhs <=> static_cast<long long>(rhs);
    }

    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
        requires(sizeof(Rhs) == sizeof(long long))
    {
        if (lhs < 0) {
            return std::strong_ordering::less;
        } else {
            return static_cast<unsigned long long>(lhs) <=> rhs;
        }
    }
};

template<std::unsigned_integral Lhs, std::signed_integral Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
        requires(sizeof(Lhs) < sizeof(long long))
    {
        return static_cast<long long>(lhs) <=> rhs;
    }

    [[nodiscard]] constexpr std::strong_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
        requires(sizeof(Lhs) == sizeof(long long))
    {
        if (rhs < 0) {
            return std::strong_ordering::greater;
        } else {
            return lhs <=> static_cast<unsigned long long>(rhs);
        }
    }
};

template<std::floating_point Lhs, std::integral Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::partial_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
    {
        if constexpr (sizeof(Rhs) < sizeof(float)) {
            // All 16-bit integers can be exactly represented as float.
            return lhs <=> static_cast<float>(rhs);
        } else if constexpr (sizeof(Rhs) < sizeof(double)) {
            // All 32-bit integers can be exactly represented as double.
            return lhs <=> static_cast<double>(rhs);
        } else {
            return lhs <=> static_cast<long double>(rhs);
        }
    }
};

template<std::integral Lhs, std::floating_point Rhs>
struct three_way_comparison<Lhs, Rhs> {
    [[nodiscard]] constexpr std::partial_ordering operator()(Lhs const& lhs, Rhs const& rhs) const noexcept
    {
        if constexpr (sizeof(Lhs) < sizeof(float)) {
            // All 16-bit integers can be exactly represented as float.
            return static_cast<float>(lhs) <=> rhs;
        } else if constexpr (sizeof(Lhs) < sizeof(double)) {
            // All 32-bit integers can be exactly represented as double.
            return static_cast<double>(lhs) <=> rhs;
        } else {
            return static_cast<long double>(lhs) <=> rhs;
        }
    }
};

/** Safely compare two arithmetic values to each other.
 *
 * @param lhs The left-hand-side arithmetic value.
 * @param rhs The right-hand-side arithmetic value.
 * @return A std::strong_ordering or std::weak_ordering of the @a lhs and @a rhs.
 */
template<arithmetic Lhs, arithmetic Rhs>
[[nodiscard]] constexpr auto three_way_compare(Lhs const& lhs, Rhs const& rhs) noexcept
{
    return three_way_comparison<Lhs, Rhs>{}(lhs, rhs);
}

}} // namespace hi::v1
