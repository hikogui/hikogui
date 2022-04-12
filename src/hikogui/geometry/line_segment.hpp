// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "point.hpp"

namespace tt::inline v1 {

class line_segment {
public:
    constexpr line_segment(line_segment const &) noexcept = default;
    constexpr line_segment(line_segment &&) noexcept = default;
    constexpr line_segment &operator=(line_segment const &) noexcept = default;
    constexpr line_segment &operator=(line_segment &&) noexcept = default;

    [[nodiscard]] constexpr line_segment(point3 p, vector3 v) noexcept : _p(p), _v(v) {}

    [[nodiscard]] constexpr line_segment(point3 p0, point3 p1) noexcept : line_segment(p0, p1 - p0) {}

    [[nodiscard]] constexpr point3 origin() const noexcept
    {
        return _p;
    }

    [[nodiscard]] constexpr vector3 direction() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr friend float hypot(line_segment const &rhs) noexcept
    {
        return hypot(rhs._v);
    }

    template<std::size_t I>
    [[nodiscard]] constexpr friend point3 get(line_segment const &rhs) noexcept
    {
        if constexpr (I == 0) {
            return _p;
        } else if constexpr (I == 1) {
            return _p + _v;
        } else {
            tt_static_no_default();
        }
    }

    [[nodiscard]] constexpr friend point3 midpoint(line_segment const &rhs) noexcept
    {
        return rhs._p + rhs._v * 0.5f;
    }

private:
    point3 _p;
    vector3 _v;
};


}

