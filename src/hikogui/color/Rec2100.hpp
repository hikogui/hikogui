// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "Rec2020.hpp"
#include <cmath>
#include <array>

namespace hi::inline v1 {

constexpr matrix3 Rec2100_to_XYZ = Rec2020_to_XYZ;

constexpr matrix3 XYZ_to_Rec2100 = XYZ_to_Rec2020;

[[nodiscard]] inline float Rec2100_linear_to_gamma(float L) noexcept
{
    constexpr float c1 = 0.8359375f;
    constexpr float c2 = 18.8515625f;
    constexpr float c3 = 18.6875f;
    constexpr float m1 = 0.1593017578125f;
    constexpr float m2 = 78.84375;

    hilet Lm1 = std::pow(L, m1);

    return std::pow((c1 + c2 * Lm1) / (1.0f + c3 * Lm1), m2);
}

[[nodiscard]] inline float Rec2100_gamma_to_linear(float N) noexcept
{
    constexpr float c1 = 0.8359375f;
    constexpr float c2 = 18.8515625f;
    constexpr float c3 = 18.6875f;
    constexpr float m1 = 0.1593017578125f;
    constexpr float m2 = 78.84375;

    hilet Nm2 = std::pow(N, 1.0f / m2);

    return std::pow((Nm2 - c1) / (c2 - c3 * Nm2), 1.0f / m1);
}

} // namespace hi::inline v1
