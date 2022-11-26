// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** geometry/margins.hpp
 * @ingroup geometry
 */

#pragma once

#include "../rapid/numeric_array.hpp"

namespace hi {
inline namespace v1 {

/** The left, bottom, right and top margins.
 * @ingroup geometry
 */
class margins {
public:
    constexpr margins(margins const &) noexcept = default;
    constexpr margins(margins &&) noexcept = default;
    constexpr margins &operator=(margins const &) noexcept = default;
    constexpr margins &operator=(margins &&) noexcept = default;

    [[nodiscard]] constexpr margins() noexcept : _v() {}
    [[nodiscard]] constexpr margins(float margin) noexcept : _v(margin, margin, margin, margin) {}
    [[nodiscard]] constexpr margins(float left, float bottom, float right, float top) noexcept : _v(left, bottom, right, top) {}
    [[nodiscard]] constexpr explicit margins(f32x4 v) noexcept : _v(v) {}

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr float const &left() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float &left() noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float const &bottom() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float &bottom() noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float const &right() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float &right() noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float const &top() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr float &top() noexcept
    {
        return _v.w();
    }

    template<int I>
    [[nodiscard]] constexpr friend float get(margins const &rhs) noexcept
    {
        return get<I>(rhs._v);
    }

    [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept
    {
        return _v[i];
    }

    [[nodiscard]] constexpr friend margins max(margins const &lhs, margins const &rhs) noexcept
    {
        return margins{max(lhs._v, rhs._v)};
    }

    [[nodiscard]] constexpr friend bool operator==(margins const &lhs, margins const &rhs) noexcept = default;

private:
    f32x4 _v;
};

}} // namespace hi::inline v1

