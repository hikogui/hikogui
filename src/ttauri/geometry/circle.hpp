
#pragma once

#include "point.hpp"
#include "aarectangle.hpp"

namespace tt::inline v1 {

class circle {
public:
    constexpr circle(circle const &other) noexcept = default;
    constexpr circle(circle &&other) noexcept = default;
    constexpr circle &operator=(circle const &other) noexcept = default;
    constexpr circle &operator=(circle &&other) noexcept = default;

    [[nodiscard]] constexpr circle(point3 point, float radius) noexcept :
        _v(f32x4{point})
    {
        _v.w() = radius;
        tt_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr circle(aarectangle square) noexcept :
    {
        tt_axiom(square.is_square());
        auto square_ = f32x4{square};

        // center=(p3 + p1)/2, radius=(p3 - p1)/2
        _v = (addsub<0b0011>(square_.xyxy(), square_.zwzw()) * 0.5f).xy0w();
        tt_axiom(holds_invariant());
    }

    [[nodiscard]] constexpr float radius() const noexcept
    {
        return _v.w();
    }

    [[nodiscard]] constexpr point3 center() const noexcept
    {
        return point3{_v.xyz1()};
    }

    [[nodiscard]] constexpr friend point3 midpoint(circle const &rhs) noexcept
    {
        return point3{rhs.center()};
    }

    [[nodiscard]] constexpr friend aarectangle bounding_rectangle(circle const &rhs) noexcept
    {
        auto p = rhs._v.xyxy();
        auto r = neg<0b0011>(rhs._v.wwww());
        return aarectangle{p + r};
    }


private:
    // Stored as a center point (x, y, z), and radius (w).
    f32x4 _v;

    [[nodiscard]] constexpr bool holds_invariant() const noexcept
    {
        return _v.w() >= 0.0f;
    }
};


}

