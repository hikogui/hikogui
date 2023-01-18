// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/identity.hpp Defines identity type.
 * @ingroup geometry
 */

#pragma once

#include "matrix.hpp"

namespace hi { inline namespace v1 {
namespace geo {

/** Identity transform.
 * @ingroup geometry
 *
 */
class identity {
public:
    constexpr identity(identity const&) noexcept = default;
    constexpr identity(identity&&) noexcept = default;
    constexpr identity& operator=(identity const&) noexcept = default;
    constexpr identity& operator=(identity&&) noexcept = default;

    constexpr identity() noexcept = default;

    template<int E>
    constexpr operator matrix<E>() const noexcept
    {
        return matrix<E>();
    }

    [[nodiscard]] identity operator~() const noexcept
    {
        return {};
    }

    template<typename O, int E>
    [[nodiscard]] constexpr vector<O, E> operator*(vector<O, E> const& rhs) const noexcept
    {
        return rhs;
    }

    template<typename O, int E>
    [[nodiscard]] constexpr point<O, E> operator*(point<O, E> const& rhs) const noexcept
    {
        return rhs;
    }

    template<typename O>
    [[nodiscard]] constexpr axis_aligned_rectangle<O> operator*(axis_aligned_rectangle<O> const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr rectangle operator*(rectangle const& rhs) const noexcept
    {
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr identity operator*(identity const&) const noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return true;
    }
};

} // namespace geo

/** 2D identity transform.
 * @ingroup geometry
 */
using identity2 = geo::identity;

/** 2D identity transform.
 * @ingroup geometry
 */
using identity3 = geo::identity;

}} // namespace hi::v1
