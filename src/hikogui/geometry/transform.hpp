// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "matrix2.hpp"
#include "matrix3.hpp"
#include "identity.hpp"
#include "translate2.hpp"
#include "translate3.hpp"
#include "rotate2.hpp"
#include "rotate3.hpp"
#include "scale2.hpp"
#include "scale3.hpp"
#include "perspective.hpp"
#include <type_traits>

namespace hi { inline namespace v1 {

[[nodiscard]] constexpr translate2 operator*(geo::identity const& lhs, translate2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr translate3 operator*(geo::identity const& lhs, translate3 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr matrix2 operator*(translate2 const& lhs, scale2 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix2{f32x4{rhs}.x000(), f32x4{rhs}._0y00(), f32x4{rhs}._00z0(), f32x4{lhs}.xyz1()};
}

[[nodiscard]] constexpr matrix3 operator*(translate3 const& lhs, scale3 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix3{f32x4{rhs}.x000(), f32x4{rhs}._0y00(), f32x4{rhs}._00z0(), f32x4{lhs}.xyz1()};
}

[[nodiscard]] constexpr matrix2 operator*(scale2 const& lhs, translate2 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix2{f32x4{lhs}.x000(), f32x4{lhs}._0y00(), f32x4{lhs}._00z0(), f32x4{lhs} * f32x4{rhs}.xyz1()};
}

[[nodiscard]] constexpr matrix3 operator*(scale3 const& lhs, translate3 const& rhs) noexcept
{
    hi_axiom(lhs.holds_invariant() && rhs.holds_invariant());
    return matrix3{f32x4{lhs}.x000(), f32x4{lhs}._0y00(), f32x4{lhs}._00z0(), f32x4{lhs} * f32x4{rhs}.xyz1()};
}

[[nodiscard]] constexpr matrix2 operator*(geo::identity const& lhs, matrix2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr matrix3 operator*(geo::identity const& lhs, matrix3 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr scale2 operator*(geo::identity const& lhs, scale2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr scale3 operator*(geo::identity const& lhs, scale3 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr rotate2 operator*(geo::identity const& lhs, rotate2 const& rhs) noexcept
{
    return rhs;
}

[[nodiscard]] constexpr rotate3 operator*(geo::identity const& lhs, rotate3 const& rhs) noexcept
{
    return rhs;
}

namespace geo {




template<typename T>
struct transform : public std::false_type {};

// clang-format off
template<> struct transform<matrix2> : public std::true_type {};
template<> struct transform<matrix3> : public std::true_type {};
template<> struct transform<identity> : public std::true_type {};
template<> struct transform<translate2> : public std::true_type {};
template<> struct transform<translate3> : public std::true_type {};
template<> struct transform<rotate2> : public std::true_type {};
template<> struct transform<rotate3> : public std::true_type {};
template<> struct transform<scale2> : public std::true_type {};
template<> struct transform<scale3> : public std::true_type {};
template<> struct transform<perspective> : public std::true_type {};
// clang-format on

template<typename T>
constexpr bool transform_v = transform<T>::value;

template<typename T>
concept transformer = transform_v<T>;

} // namespace geo
}} // namespace hi::v1
