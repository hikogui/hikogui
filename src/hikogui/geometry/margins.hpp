// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** geometry/margins.hpp
 * @ingroup geometry
 */

#pragma once

#include "../SIMD/module.hpp"
#include "../utility/module.hpp"

namespace hi { inline namespace v1 {
namespace geo {

/** The left, bottom, right and top margins.
 * @ingroup geometry
 */
template<typename T>
class margins {
public:
    using value_type = T;
    using array_type = simd<T, 4>;

    constexpr margins(margins const&) noexcept = default;
    constexpr margins(margins&&) noexcept = default;
    constexpr margins& operator=(margins const&) noexcept = default;
    constexpr margins& operator=(margins&&) noexcept = default;

    [[nodiscard]] constexpr margins() noexcept : _v() {}
    [[nodiscard]] constexpr margins(value_type margin) noexcept : _v(margin, margin, margin, margin) {}
    [[nodiscard]] constexpr margins(value_type left, value_type bottom, value_type right, value_type top) noexcept :
        _v(left, bottom, right, top)
    {
    }
    [[nodiscard]] constexpr explicit margins(array_type v) noexcept : _v(v) {}

    [[nodiscard]] constexpr explicit operator array_type() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr value_type left() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type& left() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr value_type bottom() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type& bottom() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr value_type right() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr value_type& right() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr value_type top() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr value_type& top() noexcept
    {
        return _v.w();
    }

    template<int I>
    [[nodiscard]] constexpr friend value_type get(margins const& rhs) noexcept
    {
        return get<I>(rhs._v);
    }

    [[nodiscard]] constexpr value_type operator[](std::size_t i) const noexcept
    {
        return _v[i];
    }

    constexpr margins& operator+=(margins const& rhs) noexcept
    {
        _v += rhs._v;
        return *this;
    }

    [[nodiscard]] constexpr friend margins max(margins const& lhs, margins const& rhs) noexcept
    {
        return margins{max(lhs._v, rhs._v)};
    }

    [[nodiscard]] constexpr friend bool operator==(margins const& lhs, margins const& rhs) noexcept
    {
        return equal(lhs._v, rhs._v);
    }

private:
    array_type _v;
};

} // namespace geo

using margins = geo::margins<float>;
using marginsi = geo::margins<int>;

template<>
[[nodiscard]] constexpr marginsi narrow_cast(margins const& rhs) noexcept
{
    return {
        narrow_cast<int>(rhs.left()), narrow_cast<int>(rhs.bottom()), narrow_cast<int>(rhs.right()), narrow_cast<int>(rhs.top())};
}

template<>
[[nodiscard]] constexpr margins narrow_cast(marginsi const& rhs) noexcept
{
    return {
        narrow_cast<float>(rhs.left()),
        narrow_cast<float>(rhs.bottom()),
        narrow_cast<float>(rhs.right()),
        narrow_cast<float>(rhs.top())};
}

}} // namespace hi::v1
