// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "float_to_half.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <limits>
#include <algorithm>

namespace float_to_half_tests {

[[nodiscard]] uint16_t float_to_half_sse4_1(float v) noexcept
{
    auto v_ = std::array<float, 4>{v, v, v, v};
    auto r = hi::float_to_half_sse4_1(v_);
    return std::get<0>(r);
}

}

TEST(float_to_half, generic)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    ASSERT_EQ(hi::float_to_half_generic(-std::numeric_limits<float>::infinity()), 0xfc00);

    // infinity
    ASSERT_EQ(hi::float_to_half_generic(std::numeric_limits<float>::infinity()), 0x7c00);

    // 65520
    ASSERT_EQ(hi::float_to_half_generic(65520.0f), 0x7BFF);

    // 65520-
    ASSERT_EQ(hi::float_to_half_generic(65519.996f), 0x7BFF);

    // 65504+
    ASSERT_EQ(hi::float_to_half_generic(65504.004f), 0x7BFF);

    // 65504-
    ASSERT_EQ(hi::float_to_half_generic(65503.996f), 0x7BFE);

    // 2^15+
    ASSERT_EQ(hi::float_to_half_generic(32768.002f), 0x7800);

    // 2^15-
    ASSERT_EQ(hi::float_to_half_generic(32767.998f), 0x77FF);

    // 32760+
    ASSERT_EQ(hi::float_to_half_generic(32760.002f), 0x77FF);

    // 32760
    ASSERT_EQ(hi::float_to_half_generic(32760.0f), 0x77FF);

    // 32760-
    ASSERT_EQ(hi::float_to_half_generic(32759.998f), 0x77FF);

    // 32752+
    ASSERT_EQ(hi::float_to_half_generic(32752.002f), 0x77FF);

    // 32752-
    ASSERT_EQ(hi::float_to_half_generic(32751.998f), 0x77FE);

    // 1027.5+
    ASSERT_EQ(hi::float_to_half_generic(1027.50012f), 0x6403);

    // 1027.5
    ASSERT_EQ(hi::float_to_half_generic(1027.5f), 0x6403);

    // 1027.5-
    ASSERT_EQ(hi::float_to_half_generic(1027.49988f), 0x6403);

    // pi
    ASSERT_EQ(hi::float_to_half_generic(3.1415927f), 0x4248);

    // e
    ASSERT_EQ(hi::float_to_half_generic(2.7182818f), 0x416F);

    // subnormal+
    ASSERT_EQ(hi::float_to_half_generic(3.07261980e-05f), 0x0203);

    // subnormal
    ASSERT_EQ(hi::float_to_half_generic(3.07261944e-05f), 0x0203);

    // subnormal-
    ASSERT_EQ(hi::float_to_half_generic(3.07261907e-05f), 0x0203);

    // 1/3
    ASSERT_EQ(hi::float_to_half_generic(0.3333333f), 0x3555);

    // 0.3
    ASSERT_EQ(hi::float_to_half_generic(0.3f), 0x34CC);

    // min_subnormal-
    ASSERT_EQ(hi::float_to_half_generic(5.9604641e-08f), 0x0000);

    // (min_subnormal/2)+
    ASSERT_EQ(hi::float_to_half_generic(2.9802325e-08f), 0x0000);

    // min_subnormal/2
    ASSERT_EQ(hi::float_to_half_generic(2.9802322e-08f), 0x0000);

    // epsilon
    ASSERT_EQ(hi::float_to_half_generic(0.0f), 0x0000);

    // -epsilon
    ASSERT_EQ(hi::float_to_half_generic(-0.0f), 0x8000);

    // -min_subnormal/2
    ASSERT_EQ(hi::float_to_half_generic(-2.9802322e-08f), 0x8000);

    // -(min_subnorm/2)-
    ASSERT_EQ(hi::float_to_half_generic(-2.9802325e-08f), 0x8000);

    // -min_subnormal+
    ASSERT_EQ(hi::float_to_half_generic(-5.9604641e-08f), 0x8000);

    // -0.3
    ASSERT_EQ(hi::float_to_half_generic(-0.3f), 0xB4CC);

    // -1/3
    ASSERT_EQ(hi::float_to_half_generic(-0.3333333f), 0xB555);

    // neg subnormal+
    ASSERT_EQ(hi::float_to_half_generic(-3.07261907e-05f), 0x8203);

    // neg subnormal
    ASSERT_EQ(hi::float_to_half_generic(-3.07261944e-05f), 0x8203);

    // neg subnormal-
    ASSERT_EQ(hi::float_to_half_generic(-3.07261980e-05f), 0x8203);

    // -e
    ASSERT_EQ(hi::float_to_half_generic(-2.7182818f), 0xC16F);

    // -pi
    ASSERT_EQ(hi::float_to_half_generic(-3.1415927f), 0xC248);

    // -1027.5+
    ASSERT_EQ(hi::float_to_half_generic(-1027.49988f), 0xE403);

    // -1027.5
    ASSERT_EQ(hi::float_to_half_generic(-1027.5f),0xE403);

    // -1027.5-
    ASSERT_EQ(hi::float_to_half_generic(-1027.50012f), 0xE403);

    // -32752+
    ASSERT_EQ(hi::float_to_half_generic(-32751.998f), 0xF7FE);

    // -32752-
    ASSERT_EQ(hi::float_to_half_generic(-32752.002f), 0xF7FF);

    // -32760+
    ASSERT_EQ(hi::float_to_half_generic(-32759.998f), 0xF7FF);

    // -32760
    ASSERT_EQ(hi::float_to_half_generic(-32760.0f), 0xF7FF);

    // -32760-
    ASSERT_EQ(hi::float_to_half_generic(-32760.002f), 0xF7FF);

    // -2^15+
    ASSERT_EQ(hi::float_to_half_generic(-32767.998f), 0xF7FF);

    // -2^15-
    ASSERT_EQ(hi::float_to_half_generic(-32768.002f), 0xF800);

    // -65504+
    ASSERT_EQ(hi::float_to_half_generic(-65503.996f), 0xFBFE);

    // -65504-
    ASSERT_EQ(hi::float_to_half_generic(-65504.004f), 0xFBFF);

    // -65520+
    ASSERT_EQ(hi::float_to_half_generic(-65519.996f), 0xFBFF);

    // -65520
    ASSERT_EQ(hi::float_to_half_generic(-65520.0f), 0xFBFF);
}

#if HI_HAS_F16C
TEST(float_to_half, f16c)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    ASSERT_EQ(hi::float_to_half_f16c(-std::numeric_limits<float>::infinity()), 0xfc00);

    // infinity
    ASSERT_EQ(hi::float_to_half_f16c(std::numeric_limits<float>::infinity()), 0x7c00);

    // 65520
    ASSERT_EQ(hi::float_to_half_f16c(65520.0f), 0x7BFF);

    // 65520-
    ASSERT_EQ(hi::float_to_half_f16c(65519.996f), 0x7BFF);

    // 65504+
    ASSERT_EQ(hi::float_to_half_f16c(65504.004f), 0x7BFF);

    // 65504-
    ASSERT_EQ(hi::float_to_half_f16c(65503.996f), 0x7BFE);

    // 2^15+
    ASSERT_EQ(hi::float_to_half_f16c(32768.002f), 0x7800);

    // 2^15-
    ASSERT_EQ(hi::float_to_half_f16c(32767.998f), 0x77FF);

    // 32760+
    ASSERT_EQ(hi::float_to_half_f16c(32760.002f), 0x77FF);

    // 32760
    ASSERT_EQ(hi::float_to_half_f16c(32760.0f), 0x77FF);

    // 32760-
    ASSERT_EQ(hi::float_to_half_f16c(32759.998f), 0x77FF);

    // 32752+
    ASSERT_EQ(hi::float_to_half_f16c(32752.002f), 0x77FF);

    // 32752-
    ASSERT_EQ(hi::float_to_half_f16c(32751.998f), 0x77FE);

    // 1027.5+
    ASSERT_EQ(hi::float_to_half_f16c(1027.50012f), 0x6403);

    // 1027.5
    ASSERT_EQ(hi::float_to_half_f16c(1027.5f), 0x6403);

    // 1027.5-
    ASSERT_EQ(hi::float_to_half_f16c(1027.49988f), 0x6403);

    // pi
    ASSERT_EQ(hi::float_to_half_f16c(3.1415927f), 0x4248);

    // e
    ASSERT_EQ(hi::float_to_half_f16c(2.7182818f), 0x416F);

    // subnormal+
    ASSERT_EQ(hi::float_to_half_f16c(3.07261980e-05f), 0x0203);

    // subnormal
    ASSERT_EQ(hi::float_to_half_f16c(3.07261944e-05f), 0x0203);

    // subnormal-
    ASSERT_EQ(hi::float_to_half_f16c(3.07261907e-05f), 0x0203);

    // 1/3
    ASSERT_EQ(hi::float_to_half_f16c(0.3333333f), 0x3555);

    // 0.3
    ASSERT_EQ(hi::float_to_half_f16c(0.3f), 0x34CC);

    // min_subnormal-
    ASSERT_EQ(hi::float_to_half_f16c(5.9604641e-08f), 0x0000);

    // (min_subnormal/2)+
    ASSERT_EQ(hi::float_to_half_f16c(2.9802325e-08f), 0x0000);

    // min_subnormal/2
    ASSERT_EQ(hi::float_to_half_f16c(2.9802322e-08f), 0x0000);

    // epsilon
    ASSERT_EQ(hi::float_to_half_f16c(0.0f), 0x0000);

    // -epsilon
    ASSERT_EQ(hi::float_to_half_f16c(-0.0f), 0x8000);

    // -min_subnormal/2
    ASSERT_EQ(hi::float_to_half_f16c(-2.9802322e-08f), 0x8000);

    // -(min_subnorm/2)-
    ASSERT_EQ(hi::float_to_half_f16c(-2.9802325e-08f), 0x8000);

    // -min_subnormal+
    ASSERT_EQ(hi::float_to_half_f16c(-5.9604641e-08f), 0x8000);

    // -0.3
    ASSERT_EQ(hi::float_to_half_f16c(-0.3f), 0xB4CC);

    // -1/3
    ASSERT_EQ(hi::float_to_half_f16c(-0.3333333f), 0xB555);

    // neg subnormal+
    ASSERT_EQ(hi::float_to_half_f16c(-3.07261907e-05f), 0x8203);

    // neg subnormal
    ASSERT_EQ(hi::float_to_half_f16c(-3.07261944e-05f), 0x8203);

    // neg subnormal-
    ASSERT_EQ(hi::float_to_half_f16c(-3.07261980e-05f), 0x8203);

    // -e
    ASSERT_EQ(hi::float_to_half_f16c(-2.7182818f), 0xC16F);

    // -pi
    ASSERT_EQ(hi::float_to_half_f16c(-3.1415927f), 0xC248);

    // -1027.5+
    ASSERT_EQ(hi::float_to_half_f16c(-1027.49988f), 0xE403);

    // -1027.5
    ASSERT_EQ(hi::float_to_half_f16c(-1027.5f),0xE403);

    // -1027.5-
    ASSERT_EQ(hi::float_to_half_f16c(-1027.50012f), 0xE403);

    // -32752+
    ASSERT_EQ(hi::float_to_half_f16c(-32751.998f), 0xF7FE);

    // -32752-
    ASSERT_EQ(hi::float_to_half_f16c(-32752.002f), 0xF7FF);

    // -32760+
    ASSERT_EQ(hi::float_to_half_f16c(-32759.998f), 0xF7FF);

    // -32760
    ASSERT_EQ(hi::float_to_half_f16c(-32760.0f), 0xF7FF);

    // -32760-
    ASSERT_EQ(hi::float_to_half_f16c(-32760.002f), 0xF7FF);

    // -2^15+
    ASSERT_EQ(hi::float_to_half_f16c(-32767.998f), 0xF7FF);

    // -2^15-
    ASSERT_EQ(hi::float_to_half_f16c(-32768.002f), 0xF800);

    // -65504+
    ASSERT_EQ(hi::float_to_half_f16c(-65503.996f), 0xFBFE);

    // -65504-
    ASSERT_EQ(hi::float_to_half_f16c(-65504.004f), 0xFBFF);

    // -65520+
    ASSERT_EQ(hi::float_to_half_f16c(-65519.996f), 0xFBFF);

    // -65520
    ASSERT_EQ(hi::float_to_half_f16c(-65520.0f), 0xFBFF);
}
#endif

#if HI_HAS_AVX2
TEST(float_to_half, sse4_1)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-std::numeric_limits<float>::infinity()), 0xfc00);

    // infinity
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(std::numeric_limits<float>::infinity()), 0x7c00);

    // 65520
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(65520.0f), 0x7BFF);

    // 65520-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(65519.996f), 0x7BFF);

    // 65504+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(65504.004f), 0x7BFF);

    // 65504-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(65503.996f), 0x7BFE);

    // 2^15+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32768.002f), 0x7800);

    // 2^15-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32767.998f), 0x77FF);

    // 32760+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32760.002f), 0x77FF);

    // 32760
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32760.0f), 0x77FF);

    // 32760-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32759.998f), 0x77FF);

    // 32752+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32752.002f), 0x77FF);

    // 32752-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(32751.998f), 0x77FE);

    // 1027.5+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(1027.50012f), 0x6403);

    // 1027.5
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(1027.5f), 0x6403);

    // 1027.5-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(1027.49988f), 0x6403);

    // pi
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(3.1415927f), 0x4248);

    // e
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(2.7182818f), 0x416F);

    // subnormal+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(3.07261980e-05f), 0x0203);

    // subnormal
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(3.07261944e-05f), 0x0203);

    // subnormal-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(3.07261907e-05f), 0x0203);

    // 1/3
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(0.3333333f), 0x3555);

    // 0.3
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(0.3f), 0x34CC);

    // min_subnormal-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(5.9604641e-08f), 0x0000);

    // (min_subnormal/2)+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(2.9802325e-08f), 0x0000);

    // min_subnormal/2
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(2.9802322e-08f), 0x0000);

    // epsilon
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(0.0f), 0x0000);

    // -epsilon
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-0.0f), 0x8000);

    // -min_subnormal/2
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-2.9802322e-08f), 0x8000);

    // -(min_subnorm/2)-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-2.9802325e-08f), 0x8000);

    // -min_subnormal+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-5.9604641e-08f), 0x8000);

    // -0.3
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-0.3f), 0xB4CC);

    // -1/3
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-0.3333333f), 0xB555);

    // neg subnormal+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-3.07261907e-05f), 0x8203);

    // neg subnormal
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-3.07261944e-05f), 0x8203);

    // neg subnormal-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-3.07261980e-05f), 0x8203);

    // -e
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-2.7182818f), 0xC16F);

    // -pi
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-3.1415927f), 0xC248);

    // -1027.5+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-1027.49988f), 0xE403);

    // -1027.5
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-1027.5f),0xE403);

    // -1027.5-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-1027.50012f), 0xE403);

    // -32752+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32751.998f), 0xF7FE);

    // -32752-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32752.002f), 0xF7FF);

    // -32760+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32759.998f), 0xF7FF);

    // -32760
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32760.0f), 0xF7FF);

    // -32760-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32760.002f), 0xF7FF);

    // -2^15+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32767.998f), 0xF7FF);

    // -2^15-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-32768.002f), 0xF800);

    // -65504+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-65503.996f), 0xFBFE);

    // -65504-
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-65504.004f), 0xFBFF);

    // -65520+
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-65519.996f), 0xFBFF);

    // -65520
    ASSERT_EQ(float_to_half_tests::float_to_half_sse4_1(-65520.0f), 0xFBFF);
}
#endif