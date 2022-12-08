// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"
#include "translate.hpp"
#include "rotate.hpp"
#include "scale.hpp"
#include "perspective.hpp"
#include <type_traits>

namespace hi::inline v1 { namespace geo {

template<int D>
[[nodiscard]] constexpr matrix<D> operator*(identity const &lhs, matrix<D> const &rhs) noexcept
{
    return rhs;
}

template<typename T, int D>
[[nodiscard]] constexpr translate<T, D> operator*(identity const &lhs, translate<T, D> const &rhs) noexcept
{
    return rhs;
}

template<int D>
[[nodiscard]] constexpr scale<D> operator*(identity const &lhs, scale<D> const &rhs) noexcept
{
    return rhs;
}

template<int D>
[[nodiscard]] constexpr rotate<D> operator*(identity const &lhs, rotate<D> const &rhs) noexcept
{
    return rhs;
}

template<int D, int E>
[[nodiscard]] constexpr auto operator*(translate<float, D> const &lhs, scale<E> const &rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix<std::max(D, E)>{
        static_cast<f32x4>(rhs).x000(),
        static_cast<f32x4>(rhs)._0y00(),
        static_cast<f32x4>(rhs)._00z0(),
        static_cast<f32x4>(lhs).xyz1()};
}

template<int D, int E>
[[nodiscard]] constexpr auto operator*(scale<D> const &lhs, translate<float, E> const &rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix<std::max(D, E)>{
        static_cast<f32x4>(lhs).x000(),
        static_cast<f32x4>(lhs)._0y00(),
        static_cast<f32x4>(lhs)._00z0(),
        static_cast<f32x4>(lhs) * static_cast<f32x4>(rhs).xyz1()};
}

template<typename T>
struct transform : public std::false_type {
};

// clang-format off
template<> struct transform<matrix<2>> : public std::true_type {};
template<> struct transform<matrix<3>> : public std::true_type {};
template<> struct transform<identity> : public std::true_type {};
template<> struct transform<translate<float, 2>> : public std::true_type {};
template<> struct transform<translate<float, 3>> : public std::true_type {};
template<> struct transform<rotate<2>> : public std::true_type {};
template<> struct transform<rotate<3>> : public std::true_type {};
template<> struct transform<scale<2>> : public std::true_type {};
template<> struct transform<scale<3>> : public std::true_type {};
template<> struct transform<perspective> : public std::true_type {};
// clang-format on

template<typename T>
constexpr bool transform_v = transform<T>::value;

template<typename T>
concept transformer = transform_v<T>;

}} // namespace hi::inline v1::geo