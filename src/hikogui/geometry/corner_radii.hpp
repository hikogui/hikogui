// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file geometry/corner_radii.hpp Defined the corner_radii type.
 * @ingroup geometry
 */

#pragma once

namespace hi { inline namespace v1 {
namespace geo {

/** The 4 radii of the corners of a quad or rectangle.
 *
 * @ingroup geometry
 */
template<typename T>
class corner_radii {
public:
    using value_type = T;
    using array_type = simd<value_type, 4>;

    constexpr corner_radii(corner_radii const&) noexcept = default;
    constexpr corner_radii(corner_radii&&) noexcept = default;
    constexpr corner_radii& operator=(corner_radii const&) noexcept = default;
    constexpr corner_radii& operator=(corner_radii&&) noexcept = default;

    [[nodiscard]] constexpr corner_radii() noexcept : corner_radii(-std::numeric_limits<value_type>::lowest()) {}
    [[nodiscard]] constexpr corner_radii(value_type radius) noexcept : _v(radius, radius, radius, radius) {}
    [[nodiscard]] constexpr corner_radii(value_type lb, value_type rb, value_type lt, value_type rt) noexcept : _v(lb, rb, lt, rt)
    {
    }

    /** Construct a corner_radii from a simd.
     *
     * @param v The 4 radii, x=left-bottom, y=right-bottom, z=left-top, w=right-top.
     */
    [[nodiscard]] constexpr explicit corner_radii(array_type v) noexcept : _v(v) {}

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr value_type left_bottom() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type right_bottom() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type left_top() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr value_type right_top() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr value_type& left_bottom() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type& right_bottom() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type& left_top() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr value_type& right_top() noexcept
    {
        return _v.w();
    }

    /** Get the corner radius by index.
     *
     * @tparam I The index; 0=left-bottom, 1=right-bottom, 2=left-top, 3=right-top.
     * @return The corner radius.
     */
    template<int I>
    [[nodiscard]] constexpr friend float get(corner_radii const& rhs) noexcept
    {
        return get<I>(rhs._v);
    }

    /** Get the corner radius by index.
     *
     * @param i The index; 0=left-bottom, 1=right-bottom, 2=left-top, 3=right-top.
     * @return The corner radius.
     */
    [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept
    {
        return _v[i];
    }

    [[nodiscard]] constexpr friend corner_radii operator+(corner_radii const& lhs, value_type rhs) noexcept
    {
        return corner_radii{array_type{lhs} + rhs};
    }

    [[nodiscard]] constexpr friend corner_radii operator-(corner_radii const& lhs, value_type rhs) noexcept
    {
        return corner_radii{array_type{lhs} - rhs};
    }

    [[nodiscard]] constexpr friend corner_radii round(corner_radii const& rhs) noexcept
    {
        return corner_radii{round(rhs._v)};
    }

    [[nodiscard]] constexpr friend corner_radii floor(corner_radii const& rhs) noexcept
    {
        return corner_radii{floor(rhs._v)};
    }

    [[nodiscard]] constexpr friend corner_radii ceil(corner_radii const& rhs) noexcept
    {
        return corner_radii{ceil(rhs._v)};
    }

private:
    array_type _v;
};

} // namespace geo

using corner_radii = geo::corner_radii<float>;

}} // namespace hi::v1
