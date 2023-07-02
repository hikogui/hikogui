// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/gidentity.hpp Defines gidentity type.
 * @ingroup geometry
 */

#pragma once

#include "matrix2.hpp"
#include "matrix3.hpp"

namespace hi { inline namespace v1 {

/** gidentity transform.
 * @ingroup geometry
 *
 */
class gidentity {
public:
    constexpr gidentity(gidentity const&) noexcept = default;
    constexpr gidentity(gidentity&&) noexcept = default;
    constexpr gidentity& operator=(gidentity const&) noexcept = default;
    constexpr gidentity& operator=(gidentity&&) noexcept = default;

    constexpr gidentity() noexcept = default;

    constexpr operator matrix2() const noexcept
    {
        return matrix2();
    }

    constexpr operator matrix3() const noexcept
    {
        return matrix3();
    }

    [[nodiscard]] gidentity operator~() const noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr vector2 operator*(vector2 const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr vector3 operator*(vector3 const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr point2 operator*(point2 const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr point3 operator*(point3 const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr aarectangle operator*(aarectangle const& rhs) const noexcept
    {
        return rhs;
    }

    [[nodiscard]] constexpr rectangle operator*(rectangle const& rhs) const noexcept
    {
        return rhs;
    }

    template<int E>
    [[nodiscard]] constexpr gidentity operator*(gidentity const&) const noexcept
    {
        return {};
    }

    [[nodiscard]] constexpr bool is_valid() const noexcept
    {
        return true;
    }
};

}} // namespace hi::v1
