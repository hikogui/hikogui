// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "float_to_half.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(float_to_half_suite) {

TEST_CASE(generic_test)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    REQUIRE(hi::float_to_half_generic(-std::numeric_limits<float>::infinity()) == 0xfc00);

    // infinity
    REQUIRE(hi::float_to_half_generic(std::numeric_limits<float>::infinity()) == 0x7c00);

    // 65520
    REQUIRE(hi::float_to_half_generic(65520.0f) == 0x7BFF);

    // 65520-
    REQUIRE(hi::float_to_half_generic(65519.996f) == 0x7BFF);

    // 65504+
    REQUIRE(hi::float_to_half_generic(65504.004f) == 0x7BFF);

    // 65504-
    REQUIRE(hi::float_to_half_generic(65503.996f) == 0x7BFE);

    // 2^15+
    REQUIRE(hi::float_to_half_generic(32768.002f) == 0x7800);

    // 2^15-
    REQUIRE(hi::float_to_half_generic(32767.998f) == 0x77FF);

    // 32760+
    REQUIRE(hi::float_to_half_generic(32760.002f) == 0x77FF);

    // 32760
    REQUIRE(hi::float_to_half_generic(32760.0f) == 0x77FF);

    // 32760-
    REQUIRE(hi::float_to_half_generic(32759.998f) == 0x77FF);

    // 32752+
    REQUIRE(hi::float_to_half_generic(32752.002f) == 0x77FF);

    // 32752-
    REQUIRE(hi::float_to_half_generic(32751.998f) == 0x77FE);

    // 1027.5+
    REQUIRE(hi::float_to_half_generic(1027.50012f) == 0x6403);

    // 1027.5
    REQUIRE(hi::float_to_half_generic(1027.5f) == 0x6403);

    // 1027.5-
    REQUIRE(hi::float_to_half_generic(1027.49988f) == 0x6403);

    // pi
    REQUIRE(hi::float_to_half_generic(3.1415927f) == 0x4248);

    // e
    REQUIRE(hi::float_to_half_generic(2.7182818f) == 0x416F);

    // subnormal+
    REQUIRE(hi::float_to_half_generic(3.07261980e-05f) == 0x0203);

    // subnormal
    REQUIRE(hi::float_to_half_generic(3.07261944e-05f) == 0x0203);

    // subnormal-
    REQUIRE(hi::float_to_half_generic(3.07261907e-05f) == 0x0203);

    // 1/3
    REQUIRE(hi::float_to_half_generic(0.3333333f) == 0x3555);

    // 0.3
    REQUIRE(hi::float_to_half_generic(0.3f) == 0x34CC);

    // min_subnormal-
    REQUIRE(hi::float_to_half_generic(5.9604641e-08f) == 0x0000);

    // (min_subnormal/2)+
    REQUIRE(hi::float_to_half_generic(2.9802325e-08f) == 0x0000);

    // min_subnormal/2
    REQUIRE(hi::float_to_half_generic(2.9802322e-08f) == 0x0000);

    // epsilon
    REQUIRE(hi::float_to_half_generic(0.0f) == 0x0000);

    // -epsilon
    REQUIRE(hi::float_to_half_generic(-0.0f) == 0x8000);

    // -min_subnormal/2
    REQUIRE(hi::float_to_half_generic(-2.9802322e-08f) == 0x8000);

    // -(min_subnorm/2)-
    REQUIRE(hi::float_to_half_generic(-2.9802325e-08f) == 0x8000);

    // -min_subnormal+
    REQUIRE(hi::float_to_half_generic(-5.9604641e-08f) == 0x8000);

    // -0.3
    REQUIRE(hi::float_to_half_generic(-0.3f) == 0xB4CC);

    // -1/3
    REQUIRE(hi::float_to_half_generic(-0.3333333f) == 0xB555);

    // neg subnormal+
    REQUIRE(hi::float_to_half_generic(-3.07261907e-05f) == 0x8203);

    // neg subnormal
    REQUIRE(hi::float_to_half_generic(-3.07261944e-05f) == 0x8203);

    // neg subnormal-
    REQUIRE(hi::float_to_half_generic(-3.07261980e-05f) == 0x8203);

    // -e
    REQUIRE(hi::float_to_half_generic(-2.7182818f) == 0xC16F);

    // -pi
    REQUIRE(hi::float_to_half_generic(-3.1415927f) == 0xC248);

    // -1027.5+
    REQUIRE(hi::float_to_half_generic(-1027.49988f) == 0xE403);

    // -1027.5
    REQUIRE(hi::float_to_half_generic(-1027.5f) == 0xE403);

    // -1027.5-
    REQUIRE(hi::float_to_half_generic(-1027.50012f) == 0xE403);

    // -32752+
    REQUIRE(hi::float_to_half_generic(-32751.998f) == 0xF7FE);

    // -32752-
    REQUIRE(hi::float_to_half_generic(-32752.002f) == 0xF7FF);

    // -32760+
    REQUIRE(hi::float_to_half_generic(-32759.998f) == 0xF7FF);

    // -32760
    REQUIRE(hi::float_to_half_generic(-32760.0f) == 0xF7FF);

    // -32760-
    REQUIRE(hi::float_to_half_generic(-32760.002f) == 0xF7FF);

    // -2^15+
    REQUIRE(hi::float_to_half_generic(-32767.998f) == 0xF7FF);

    // -2^15-
    REQUIRE(hi::float_to_half_generic(-32768.002f) == 0xF800);

    // -65504+
    REQUIRE(hi::float_to_half_generic(-65503.996f) == 0xFBFE);

    // -65504-
    REQUIRE(hi::float_to_half_generic(-65504.004f) == 0xFBFF);

    // -65520+
    REQUIRE(hi::float_to_half_generic(-65519.996f) == 0xFBFF);

    // -65520
    REQUIRE(hi::float_to_half_generic(-65520.0f) == 0xFBFF);
}

#if HI_HAS_F16C
TEST_CASE(f16c_test)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    REQUIRE(hi::float_to_half_f16c(-std::numeric_limits<float>::infinity()) == 0xfc00);

    // infinity
    REQUIRE(hi::float_to_half_f16c(std::numeric_limits<float>::infinity()) == 0x7c00);

    // 65520
    REQUIRE(hi::float_to_half_f16c(65520.0f) == 0x7BFF);

    // 65520-
    REQUIRE(hi::float_to_half_f16c(65519.996f) == 0x7BFF);

    // 65504+
    REQUIRE(hi::float_to_half_f16c(65504.004f) == 0x7BFF);

    // 65504-
    REQUIRE(hi::float_to_half_f16c(65503.996f) == 0x7BFE);

    // 2^15+
    REQUIRE(hi::float_to_half_f16c(32768.002f) == 0x7800);

    // 2^15-
    REQUIRE(hi::float_to_half_f16c(32767.998f) == 0x77FF);

    // 32760+
    REQUIRE(hi::float_to_half_f16c(32760.002f) == 0x77FF);

    // 32760
    REQUIRE(hi::float_to_half_f16c(32760.0f) == 0x77FF);

    // 32760-
    REQUIRE(hi::float_to_half_f16c(32759.998f) == 0x77FF);

    // 32752+
    REQUIRE(hi::float_to_half_f16c(32752.002f) == 0x77FF);

    // 32752-
    REQUIRE(hi::float_to_half_f16c(32751.998f) == 0x77FE);

    // 1027.5+
    REQUIRE(hi::float_to_half_f16c(1027.50012f) == 0x6403);

    // 1027.5
    REQUIRE(hi::float_to_half_f16c(1027.5f) == 0x6403);

    // 1027.5-
    REQUIRE(hi::float_to_half_f16c(1027.49988f) == 0x6403);

    // pi
    REQUIRE(hi::float_to_half_f16c(3.1415927f) == 0x4248);

    // e
    REQUIRE(hi::float_to_half_f16c(2.7182818f) == 0x416F);

    // subnormal+
    REQUIRE(hi::float_to_half_f16c(3.07261980e-05f) == 0x0203);

    // subnormal
    REQUIRE(hi::float_to_half_f16c(3.07261944e-05f) == 0x0203);

    // subnormal-
    REQUIRE(hi::float_to_half_f16c(3.07261907e-05f) == 0x0203);

    // 1/3
    REQUIRE(hi::float_to_half_f16c(0.3333333f) == 0x3555);

    // 0.3
    REQUIRE(hi::float_to_half_f16c(0.3f) == 0x34CC);

    // min_subnormal-
    REQUIRE(hi::float_to_half_f16c(5.9604641e-08f) == 0x0000);

    // (min_subnormal/2)+
    REQUIRE(hi::float_to_half_f16c(2.9802325e-08f) == 0x0000);

    // min_subnormal/2
    REQUIRE(hi::float_to_half_f16c(2.9802322e-08f) == 0x0000);

    // epsilon
    REQUIRE(hi::float_to_half_f16c(0.0f) == 0x0000);

    // -epsilon
    REQUIRE(hi::float_to_half_f16c(-0.0f) == 0x8000);

    // -min_subnormal/2
    REQUIRE(hi::float_to_half_f16c(-2.9802322e-08f) == 0x8000);

    // -(min_subnorm/2)-
    REQUIRE(hi::float_to_half_f16c(-2.9802325e-08f) == 0x8000);

    // -min_subnormal+
    REQUIRE(hi::float_to_half_f16c(-5.9604641e-08f) == 0x8000);

    // -0.3
    REQUIRE(hi::float_to_half_f16c(-0.3f) == 0xB4CC);

    // -1/3
    REQUIRE(hi::float_to_half_f16c(-0.3333333f) == 0xB555);

    // neg subnormal+
    REQUIRE(hi::float_to_half_f16c(-3.07261907e-05f) == 0x8203);

    // neg subnormal
    REQUIRE(hi::float_to_half_f16c(-3.07261944e-05f) == 0x8203);

    // neg subnormal-
    REQUIRE(hi::float_to_half_f16c(-3.07261980e-05f) == 0x8203);

    // -e
    REQUIRE(hi::float_to_half_f16c(-2.7182818f) == 0xC16F);

    // -pi
    REQUIRE(hi::float_to_half_f16c(-3.1415927f) == 0xC248);

    // -1027.5+
    REQUIRE(hi::float_to_half_f16c(-1027.49988f) == 0xE403);

    // -1027.5
    REQUIRE(hi::float_to_half_f16c(-1027.5f) == 0xE403);

    // -1027.5-
    REQUIRE(hi::float_to_half_f16c(-1027.50012f) == 0xE403);

    // -32752+
    REQUIRE(hi::float_to_half_f16c(-32751.998f) == 0xF7FE);

    // -32752-
    REQUIRE(hi::float_to_half_f16c(-32752.002f) == 0xF7FF);

    // -32760+
    REQUIRE(hi::float_to_half_f16c(-32759.998f) == 0xF7FF);

    // -32760
    REQUIRE(hi::float_to_half_f16c(-32760.0f) == 0xF7FF);

    // -32760-
    REQUIRE(hi::float_to_half_f16c(-32760.002f) == 0xF7FF);

    // -2^15+
    REQUIRE(hi::float_to_half_f16c(-32767.998f) == 0xF7FF);

    // -2^15-
    REQUIRE(hi::float_to_half_f16c(-32768.002f) == 0xF800);

    // -65504+
    REQUIRE(hi::float_to_half_f16c(-65503.996f) == 0xFBFE);

    // -65504-
    REQUIRE(hi::float_to_half_f16c(-65504.004f) == 0xFBFF);

    // -65520+
    REQUIRE(hi::float_to_half_f16c(-65519.996f) == 0xFBFF);

    // -65520
    REQUIRE(hi::float_to_half_f16c(-65520.0f) == 0xFBFF);
}
#endif

#if HI_HAS_SSE2
TEST_CASE(sse2_test)
{
    // Thanks to https://github.com/ecorm for the list of test vectors.

    // (-) minus infinity
    REQUIRE(hi::float_to_half_sse2(-std::numeric_limits<float>::infinity()) == 0xfc00);

    // infinity
    REQUIRE(hi::float_to_half_sse2(std::numeric_limits<float>::infinity()) == 0x7c00);

    // 65520
    REQUIRE(hi::float_to_half_sse2(65520.0f) == 0x7BFF);

    // 65520-
    REQUIRE(hi::float_to_half_sse2(65519.996f) == 0x7BFF);

    // 65504+
    REQUIRE(hi::float_to_half_sse2(65504.004f) == 0x7BFF);

    // 65504-
    REQUIRE(hi::float_to_half_sse2(65503.996f) == 0x7BFE);

    // 2^15+
    REQUIRE(hi::float_to_half_sse2(32768.002f) == 0x7800);

    // 2^15-
    REQUIRE(hi::float_to_half_sse2(32767.998f) == 0x77FF);

    // 32760+
    REQUIRE(hi::float_to_half_sse2(32760.002f) == 0x77FF);

    // 32760
    REQUIRE(hi::float_to_half_sse2(32760.0f) == 0x77FF);

    // 32760-
    REQUIRE(hi::float_to_half_sse2(32759.998f) == 0x77FF);

    // 32752+
    REQUIRE(hi::float_to_half_sse2(32752.002f) == 0x77FF);

    // 32752-
    REQUIRE(hi::float_to_half_sse2(32751.998f) == 0x77FE);

    // 1027.5+
    REQUIRE(hi::float_to_half_sse2(1027.50012f) == 0x6403);

    // 1027.5
    REQUIRE(hi::float_to_half_sse2(1027.5f) == 0x6403);

    // 1027.5-
    REQUIRE(hi::float_to_half_sse2(1027.49988f) == 0x6403);

    // pi
    REQUIRE(hi::float_to_half_sse2(3.1415927f) == 0x4248);

    // e
    REQUIRE(hi::float_to_half_sse2(2.7182818f) == 0x416F);

    // subnormal+
    REQUIRE(hi::float_to_half_sse2(3.07261980e-05f) == 0x0203);

    // subnormal
    REQUIRE(hi::float_to_half_sse2(3.07261944e-05f) == 0x0203);

    // subnormal-
    REQUIRE(hi::float_to_half_sse2(3.07261907e-05f) == 0x0203);

    // 1/3
    REQUIRE(hi::float_to_half_sse2(0.3333333f) == 0x3555);

    // 0.3
    REQUIRE(hi::float_to_half_sse2(0.3f) == 0x34CC);

    // min_subnormal-
    REQUIRE(hi::float_to_half_sse2(5.9604641e-08f) == 0x0000);

    // (min_subnormal/2)+
    REQUIRE(hi::float_to_half_sse2(2.9802325e-08f) == 0x0000);

    // min_subnormal/2
    REQUIRE(hi::float_to_half_sse2(2.9802322e-08f) == 0x0000);

    // epsilon
    REQUIRE(hi::float_to_half_sse2(0.0f) == 0x0000);

    // -epsilon
    REQUIRE(hi::float_to_half_sse2(-0.0f) == 0x8000);

    // -min_subnormal/2
    REQUIRE(hi::float_to_half_sse2(-2.9802322e-08f) == 0x8000);

    // -(min_subnorm/2)-
    REQUIRE(hi::float_to_half_sse2(-2.9802325e-08f) == 0x8000);

    // -min_subnormal+
    REQUIRE(hi::float_to_half_sse2(-5.9604641e-08f) == 0x8000);

    // -0.3
    REQUIRE(hi::float_to_half_sse2(-0.3f) == 0xB4CC);

    // -1/3
    REQUIRE(hi::float_to_half_sse2(-0.3333333f) == 0xB555);

    // neg subnormal+
    REQUIRE(hi::float_to_half_sse2(-3.07261907e-05f) == 0x8203);

    // neg subnormal
    REQUIRE(hi::float_to_half_sse2(-3.07261944e-05f) == 0x8203);

    // neg subnormal-
    REQUIRE(hi::float_to_half_sse2(-3.07261980e-05f) == 0x8203);

    // -e
    REQUIRE(hi::float_to_half_sse2(-2.7182818f) == 0xC16F);

    // -pi
    REQUIRE(hi::float_to_half_sse2(-3.1415927f) == 0xC248);

    // -1027.5+
    REQUIRE(hi::float_to_half_sse2(-1027.49988f) == 0xE403);

    // -1027.5
    REQUIRE(hi::float_to_half_sse2(-1027.5f) == 0xE403);

    // -1027.5-
    REQUIRE(hi::float_to_half_sse2(-1027.50012f) == 0xE403);

    // -32752+
    REQUIRE(hi::float_to_half_sse2(-32751.998f) == 0xF7FE);

    // -32752-
    REQUIRE(hi::float_to_half_sse2(-32752.002f) == 0xF7FF);

    // -32760+
    REQUIRE(hi::float_to_half_sse2(-32759.998f) == 0xF7FF);

    // -32760
    REQUIRE(hi::float_to_half_sse2(-32760.0f) == 0xF7FF);

    // -32760-
    REQUIRE(hi::float_to_half_sse2(-32760.002f) == 0xF7FF);

    // -2^15+
    REQUIRE(hi::float_to_half_sse2(-32767.998f) == 0xF7FF);

    // -2^15-
    REQUIRE(hi::float_to_half_sse2(-32768.002f) == 0xF800);

    // -65504+
    REQUIRE(hi::float_to_half_sse2(-65503.996f) == 0xFBFE);

    // -65504-
    REQUIRE(hi::float_to_half_sse2(-65504.004f) == 0xFBFF);

    // -65520+
    REQUIRE(hi::float_to_half_sse2(-65519.996f) == 0xFBFF);

    // -65520
    REQUIRE(hi::float_to_half_sse2(-65520.0f) == 0xFBFF);
}
#endif

}; // TEST_SUITE(float_to_half_suite)
