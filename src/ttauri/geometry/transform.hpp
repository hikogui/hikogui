// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix.hpp"
#include "identity.hpp"
#include "translate.hpp"
#include "rotate.hpp"
#include "scale.hpp"
#include <type_traits>

namespace tt::geo {

template<int D>
[[nodiscard]] constexpr matrix<D> operator*(identity const &lhs, matrix<D> const &rhs) noexcept
{
    return rhs;
}

template<int D>
[[nodiscard]] constexpr translate<D> operator*(identity const &lhs, translate<D> const &rhs) noexcept
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
[[nodiscard]] constexpr auto operator*(translate<D> const &lhs, scale<E> const &rhs) noexcept
{
    tt_axiom(lhs.is_valid() && rhs.is_valid());
    return matrix<std::max(D,E)>{
        static_cast<f32x4>(rhs).x000(),
        static_cast<f32x4>(rhs)._0y00(),
        static_cast<f32x4>(rhs)._00z0(),
        static_cast<f32x4>(lhs).xyz1()};
}

template<int D, int E>
[[nodiscard]] constexpr auto operator*(scale<D> const &lhs, translate<E> const &rhs) noexcept
{
    tt_axiom(lhs.is_valid() && rhs.is_valid());
    return matrix<std::max(D, E)>{
        static_cast<f32x4>(lhs).x000(),
        static_cast<f32x4>(lhs)._0y00(),
        static_cast<f32x4>(lhs)._00z0(),
        static_cast<f32x4>(lhs) * static_cast<f32x4>(rhs).xyz1()};
}

template<typename T, int D>
struct transform : public std::false_type {
};

template<int D>
struct transform<matrix<D>,D> : public std::true_type {
};

template<int D>
struct transform<translate<D>, D> : public std::true_type {
};

template<int D>
struct transform<rotate<D>, D> : public std::true_type {
};

template<int D>
struct transform<scale<D>, D> : public std::true_type {
};

template<typename T, int D>
constexpr bool transform_v = transform<T, D>::value;

template<typename T, int D>
concept transforming = transform_v<T, D>;

} // namespace tt
