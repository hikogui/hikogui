// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {

class corner_shapes {
public:
    constexpr corner_shapes(corner_shapes const &) noexcept = default;
    constexpr corner_shapes(corner_shapes &&) noexcept = default;
    constexpr corner_shapes &operator=(corner_shapes const &) noexcept = default;
    constexpr corner_shapes &operator=(corner_shapes &&) noexcept = default;

    [[nodiscard]] explicit constexpr corner_shapes() noexcept : _v() {}
    [[nodiscard]] explicit constexpr corner_shapes(float radius) noexcept : _v(radius, radius, radius, radius) {}
    [[nodiscard]] explicit constexpr corner_shapes(float lb, float rb, float lt, float rt) noexcept : _v(lb, rb, lt, rt) {}

    [[nodiscard]] explicit constexpr operator f32x4() const noexcept
    {
        return _v;
    }

    [[nodiscard]] constexpr friend corner_shapes operator+(corner_shapes const &lhs, float rhs) noexcept
    {
        auto r = corner_shapes{};

        for (size_t i = 0; i != lhs._v.size(); ++i) {
            if (lhs._v[i] >= 0) {
                r._v[i] = std::max(0.0f, lhs._v[i] + rhs);
            } else {
                r._v[i] = std::min(0.0f, lhs._v[i] - rhs);
            }
        }

        return r;
    }

    [[nodiscard]] constexpr friend corner_shapes operator-(corner_shapes const &lhs, float rhs) noexcept
    {
        return lhs + -rhs;
    }

private:
    f32x4 _v;
};


}
