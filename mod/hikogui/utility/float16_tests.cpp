// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "half.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <algorithm>

TEST(half, half_to_float)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // half infinity is one beyond the half range.
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x7C00)) == 65536.0f);

    // Largest normal.
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x7bff)) == 65504.0f);

    // (+) 2^15
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x7800)) == 32768.0f);

    // (+) 1/2 max normal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x77FF)) == 32752.0f);

    // (+) approx pi
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x4248)) == 3.140625f);

    // (+) approx e
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x4170)) == 2.71875f);

    // (+) two
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x4000)) == 2.0f);

    // (+) smallest > 1.0
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x3C01)) == 1.0009766f);

    // (+) one
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x3C00)) == 1.0f);

    // (+) largest < 1.0
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x3BFF)) == 0.9995117f);

    // (+) approx 2/3
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x3956)) == 0.6669922f);

    // (+) approx 1/3
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x3555)) == 0.33325195f);

    // (+) smallest normal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x0400)) == 6.1035156E-05f);

    // (+) largest subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x03FF)) == 0.0f);

    // (+) middle subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x0200)) == 0.0f);

    // (+) just below mid-subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x01FF)) == 0.0f);

    // (+) smallest subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x0001)) == 0.0f);

    // (+) positive zero
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x0000)) == 0.0f);

    // (-) negative zero
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x8000)) == -0.0f);

    // (-) highest subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x8001)) == -0.0f);

    // (-) just above mid-subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x81FF)) == -0.0f);

    // (-) middle subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x8200)) == -0.0f);

    // (-) lowest subnormal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x83FF)) == -0.0f);

    // (-) highest normal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0x8400)) == -6.1035156E-05f);

    // (-) approx -1/3
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xB555)) == -0.33325195f);

    // (-) approx -2/3
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xB956)) == -0.6669922f);

    // (-) lowest > -1.0
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xBBFF)) == -0.9995117f);

    // (-) minus one
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xBC00)) == -1.0f);

    // (-) highest < -1.0
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xBC01)) == -1.0009766f);

    // (-) minus two
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xC000)) == -2.0f);

    // (-) approx e
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xC170)) == -2.71875f);

    // (-) approx pi
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xC248)) == -3.140625f);

    // (-) 1/2 lowest normal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xF7FF)) == -32752.0f);

    // (-) 2^15
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xF800)) == -32768.0f);

    // (-) lowest normal
    static_assert(static_cast<float>(hi::half(hi::intrinsic, 0xFBFF)) == -65504.0f);
}

TEST(half, float_to_half)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    static_assert(hi::half(-std::numeric_limits<float>::infinity()).intrinsic() == 0xfc00);

    // infinity
    static_assert(hi::half(std::numeric_limits<float>::infinity()).intrinsic() == 0x7c00);

    // 65520
    static_assert(hi::half(65520.0f).intrinsic() == 0x7BFF);

    // 65520-
    static_assert(hi::half(65519.996f).intrinsic() == 0x7BFF);

    // 65504+
    static_assert(hi::half(65504.004f).intrinsic() == 0x7BFF);

    // 65504-
    static_assert(hi::half(65503.996f).intrinsic() == 0x7BFE);

    // 2^15+
    static_assert(hi::half(32768.002f).intrinsic() == 0x7800);

    // 2^15-
    static_assert(hi::half(32767.998f).intrinsic() == 0x77FF);

    // 32760+
    static_assert(hi::half(32760.002f).intrinsic() == 0x77FF);

    // 32760
    static_assert(hi::half(32760.0f).intrinsic() == 0x77FF);

    // 32760-
    static_assert(hi::half(32759.998f).intrinsic() == 0x77FF);

    // 32752+
    static_assert(hi::half(32752.002f).intrinsic() == 0x77FF);

    // 32752-
    static_assert(hi::half(32751.998f).intrinsic() == 0x77FE);

    // 1027.5+
    static_assert(hi::half(1027.50012f).intrinsic() == 0x6403);

    // 1027.5
    static_assert(hi::half(1027.5f).intrinsic() == 0x6403);

    // 1027.5-
    static_assert(hi::half(1027.49988f).intrinsic() == 0x6403);

    // pi
    static_assert(hi::half(3.1415927f).intrinsic() == 0x4248);

    // e
    static_assert(hi::half(2.7182818f).intrinsic() == 0x416F);

    // subnormal+
    static_assert(hi::half(3.07261980e-05).intrinsic() == 0x0000);

    // subnormal
    static_assert(hi::half(3.07261944e-05).intrinsic() == 0x0000);

    // subnormal-
    static_assert(hi::half(3.07261907e-05).intrinsic() == 0x0000);

    // 1/3
    static_assert(hi::half(0.3333333f).intrinsic() == 0x3555);

    // 0.3
    static_assert(hi::half(0.3f).intrinsic() == 0x34CC);

    // min_subnormal-
    static_assert(hi::half(5.9604641e-08f).intrinsic() == 0x0000);

    // (min_subnormal/2)+
    static_assert(hi::half(2.9802325e-08f).intrinsic() == 0x0000);

    // min_subnormal/2
    static_assert(hi::half(2.9802322e-08f).intrinsic() == 0x0000);

    // epsilon
    static_assert(hi::half(0.0f).intrinsic() == 0x0000);

    // -epsilon
    static_assert(hi::half(-0.0f).intrinsic() == 0x8000);

    // -min_subnormal/2
    static_assert(hi::half(-2.9802322e-08f).intrinsic() == 0x8000);

    // -(min_subnorm/2)-
    static_assert(hi::half(-2.9802325e-08f).intrinsic() == 0x8000);

    // -min_subnormal+
    static_assert(hi::half(-5.9604641e-08f).intrinsic() == 0x8000);

    // -0.3
    static_assert(hi::half(-0.3f).intrinsic() == 0xB4CC);

    // -1/3
    static_assert(hi::half(-0.3333333f).intrinsic() == 0xB555);

    // neg subnormal+
    static_assert(hi::half(-3.07261907e-05).intrinsic() == 0x8000);

    // neg subnormal
    static_assert(hi::half(-3.07261944e-05).intrinsic() == 0x8000);

    // neg subnormal-
    static_assert(hi::half(-3.07261980e-05).intrinsic() == 0x8000);

    // -e
    static_assert(hi::half(-2.7182818f).intrinsic() == 0xC16F);

    // -pi
    static_assert(hi::half(-3.1415927f).intrinsic() == 0xC248);

    // -1027.5+
    static_assert(hi::half(-1027.49988f).intrinsic() == 0xE403);

    // -1027.5
    static_assert(hi::half(-1027.5f).intrinsic() == 0xE403);

    // -1027.5-
    static_assert(hi::half(-1027.50012f).intrinsic() == 0xE403);

    // -32752+
    static_assert(hi::half(-32751.998f).intrinsic() == 0xF7FE);

    // -32752-
    static_assert(hi::half(-32752.002f).intrinsic() == 0xF7FF);

    // -32760+
    static_assert(hi::half(-32759.998f).intrinsic() == 0xF7FF);

    // -32760
    static_assert(hi::half(-32760.0f).intrinsic() == 0xF7FF);

    // -32760-
    static_assert(hi::half(-32760.002f).intrinsic() == 0xF7FF);

    // -2^15+
    static_assert(hi::half(-32767.998f).intrinsic() == 0xF7FF);

    // -2^15-
    static_assert(hi::half(-32768.002f).intrinsic() == 0xF800);

    // -65504+
    static_assert(hi::half(-65503.996f).intrinsic() == 0xFBFE);

    // -65504-
    static_assert(hi::half(-65504.004f).intrinsic() == 0xFBFF);

    // -65520+
    static_assert(hi::half(-65519.996f).intrinsic() == 0xFBFF);

    // -65520
    static_assert(hi::half(-65520.0f).intrinsic() == 0xFBFF);
}
