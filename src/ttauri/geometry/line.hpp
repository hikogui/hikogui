
#pragma once

#include "point.hpp"

namespace tt::inline v1 {

class line {
public:
    point3 p0;
    point3 p1;

    constexpr line(line const &) noexcept = default;
    constexpr line(line &&) noexcept = default;
    constexpr line &operator=(line const &) noexcept = default;
    constexpr line &operator=(line &&) noexcept = default;

    [[nodiscard]] constexpr line(point3 p0, point3 p1) noexcept : p0(p0), p1(p1) {}

    [[nodiscard]] constexpr friend point3 midpoint(line const &rhs) noexcept
    {
        return midpoint(p0, p1);
    }
};


}

