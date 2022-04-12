// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace hi::inline v1 {

class corner_radii {
public:
    constexpr corner_radii(corner_radii const &) noexcept = default;
    constexpr corner_radii(corner_radii &&) noexcept = default;
    constexpr corner_radii &operator=(corner_radii const &) noexcept = default;
    constexpr corner_radii &operator=(corner_radii &&) noexcept = default;

    [[nodiscard]] constexpr corner_radii() noexcept : corner_radii(-std::numeric_limits<float>::infinity()) {}
    [[nodiscard]] constexpr corner_radii(float radius) noexcept : _v(radius, radius, radius, radius) {}
    [[nodiscard]] constexpr corner_radii(float lb, float rb, float lt, float rt) noexcept : _v(lb, rb, lt, rt) {}
    [[nodiscard]] constexpr explicit corner_radii(f32x4 v) noexcept : _v(v) {}

    [[nodiscard]] constexpr explicit operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr float left_bottom() const noexcept
    {
        return _v.x();
    }

    [[nodiscard]] constexpr float right_bottom() const noexcept
    {
        return _v.y();
    }

    [[nodiscard]] constexpr float left_top() const noexcept
    {
        return _v.z();
    }

    [[nodiscard]] constexpr float right_top() const noexcept
    {
        return _v.w();
    }

    template<int I>
    [[nodiscard]] constexpr friend float get(corner_radii const &rhs) noexcept
    {
        return get<I>(rhs._v);
    }

    [[nodiscard]] constexpr float operator[](std::size_t i) const noexcept
    {
        return _v[i];
    }

    [[nodiscard]] constexpr friend corner_radii operator+(corner_radii const &lhs, float rhs) noexcept
    {
        return corner_radii{f32x4{lhs} + rhs};
        //auto r = corner_radii{};
        //
        //for (std::size_t i = 0; i != lhs._v.size(); ++i) {
        //    if (lhs._v[i] >= 0) {
        //        r._v[i] = std::max(0.0f, lhs._v[i] + rhs);
        //    } else {
        //        r._v[i] = std::min(0.0f, lhs._v[i] - rhs);
        //    }
        //}
        //
        //return r;
    }

    [[nodiscard]] constexpr friend corner_radii operator-(corner_radii const &lhs, float rhs) noexcept
    {
        return corner_radii{f32x4{lhs} - rhs};
        //return lhs + -rhs;
    }

private:
    f32x4 _v;
};

} // namespace hi::inline v1
