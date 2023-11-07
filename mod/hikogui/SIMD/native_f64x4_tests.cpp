// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "native_f64x4_avx.hpp"
#include "simd_intf.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>

hi_warning_push();
// C26474: Don't cast between pointer types when the conversion could be implicit (type.1).
// For the test we need to do this explicit.
hi_warning_ignore_msvc(26474);

namespace native_f64x4_tests {

// The helper function for {ASSERT|EXPECT}_EQ.
template<typename T, std::size_t N>
::testing::AssertionResult
CmpHelperEQ(const char *lhs_expression, const char *rhs_expression, ::hi::simd<T, N> const& lhs, ::hi::simd<T, N> const& rhs)
{
    if (equal(lhs, rhs)) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::internal::CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

template<typename T, std::size_t N>
::testing::AssertionResult
CmpHelperNE(const char *lhs_expression, const char *rhs_expression, ::hi::simd<T, N> const& lhs, ::hi::simd<T, N> const& rhs)
{
    if (not equal(lhs, rhs)) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::internal::CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

// The helper function for {ASSERT|EXPECT}_EQ.
template<typename T, std::size_t N>
::testing::AssertionResult CmpHelperEQ(
    const char *lhs_expression,
    const char *rhs_expression,
    ::hi::native_simd<T, N> const& lhs,
    ::hi::native_simd<T, N> const& rhs)
{
    if (equal(lhs, rhs)) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::internal::CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

template<typename T, std::size_t N>
::testing::AssertionResult CmpHelperNE(
    const char *lhs_expression,
    const char *rhs_expression,
    ::hi::native_simd<T, N> const& lhs,
    ::hi::native_simd<T, N> const& rhs)
{
    if (not equal(lhs, rhs)) {
        return ::testing::AssertionSuccess();
    }

    return ::testing::internal::CmpHelperEQFailure(lhs_expression, rhs_expression, lhs, rhs);
}

} // namespace native_f64x4_tests

#define HI_ASSERT_SIMD_EQ(val1, val2) ASSERT_PRED_FORMAT2(native_f64x4_tests::CmpHelperEQ, val1, val2)
#define HI_ASSERT_SIMD_NE(val1, val2) ASSERT_PRED_FORMAT2(native_f64x4_tests::CmpHelperNE, val1, val2)

using S = hi::native_simd<double, 4>;
using A = S::array_type;

TEST(native_f64x4, construct)
{
    {
        auto expected = A{0.0, 0.0, 0.0, 0.0};
        ASSERT_EQ(static_cast<A>(S{}), expected);
    }

    {
        auto expected = A{1.0, 0.0, 0.0, 0.0};
        ASSERT_EQ(static_cast<A>(S{1.0}), expected);
    }

    {
        auto expected = A{1.0, 2.0, 3.0, 4.0};
        ASSERT_EQ(static_cast<A>(S{1.0, 2.0, 3.0, 4.0}), expected);
    }

    {
        auto expected = A{4.0, 4.0, 4.0, 4.0};
        ASSERT_EQ(static_cast<A>(S::broadcast(4.0)), expected);
    }

    {
        auto from = A{1.0, 2.0, 3.0, 4.0};
        auto expected = A{1.0, 2.0, 3.0, 4.0};
        ASSERT_EQ(static_cast<A>(S{from}), expected);
    }

    {
        auto from = A{1.0, 2.0, 3.0, 4.0};
        auto expected = A{1.0, 2.0, 3.0, 4.0};
        ASSERT_EQ(static_cast<A>(S{from.data()}), expected);
    }

    {
        auto from = A{1.0, 2.0, 3.0, 4.0};
        auto expected = A{1.0, 2.0, 3.0, 4.0};
        ASSERT_EQ(static_cast<A>(S{static_cast<void *>(from.data())}), expected);
    }

    {
        auto a = A{1.0, 2.0, 3.0, 4.0};
        auto from = std::span(a.data(), a.size());
        auto expected = A{1.0, 2.0, 3.0, 4.0};
        ASSERT_EQ(static_cast<A>(S{from}), expected);
    }
}

TEST(native_f64x4, conversion)
{
    auto a = S{1.0, 2.0, 3.0, 4.0};
    auto expected = A{1.0, 2.0, 3.0, 4.0};

    {
        auto result = A{};
        a.store(result);
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        auto result_span = std::span(result.data(), result.size());
        a.store(result_span);
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        a.store(result.data());
        ASSERT_EQ(result, expected);
    }

    {
        auto result = A{};
        a.store(static_cast<void *>(result.data()));
        ASSERT_EQ(result, expected);
    }
}

TEST(native_f64x4, compare)
{
    HI_ASSERT_SIMD_EQ(
        S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0), S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0));
    HI_ASSERT_SIMD_NE(
        S(1.1, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0), S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0));
    HI_ASSERT_SIMD_NE(
        S(1.1, 2.1, -std::numeric_limits<double>::quiet_NaN(), -4.1),
        S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0));

    ASSERT_TRUE(almost_equal(S(1.0, 2.0, 0.5, -4.0), S(1.0, 2.0, 0.5, -4.0)));
    ASSERT_TRUE(almost_equal(S(1.00001, 2.0, 0.499999, -4.0), S(1.0, 2.00001, 0.5, -3.99999), 0.00002));

    ASSERT_EQ(
        (S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0) ==
         S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0))
            .mask(),
        0b1011);
    ASSERT_EQ(
        (S(1.1, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0) ==
         S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0))
            .mask(),
        0b1010);

    ASSERT_EQ(
        (S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0) !=
         S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0))
            .mask(),
        0b0100);
    ASSERT_EQ(
        (S(1.1, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0) !=
         S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), -4.0))
            .mask(),
        0b0101);

    ASSERT_EQ((S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), 4.0) < S(2.0, 2.0, 2.0, 2.0)).mask(), 0b0001);
    ASSERT_EQ((S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), 4.0) <= S(2.0, 2.0, 2.0, 2.0)).mask(), 0b0011);
    ASSERT_EQ((S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), 4.0) > S(2.0, 2.0, 2.0, 2.0)).mask(), 0b1000);
    ASSERT_EQ((S(1.0, 2.0, std::numeric_limits<double>::quiet_NaN(), 4.0) >= S(2.0, 2.0, 2.0, 2.0)).mask(), 0b1010);
}

TEST(native_f64x4, math)
{
    HI_ASSERT_SIMD_EQ(-S(0.0, 2.0, 3.0, 42.0), S(0.0, -2.0, -3.0, -42.0));
    HI_ASSERT_SIMD_EQ(+S(0.0, 2.0, 3.0, 42.0), S(0.0, 2.0, 3.0, 42.0));
    HI_ASSERT_SIMD_EQ(S(0.0, 2.0, 3.0, 42.0) + S(1.0, 4.0, -3.0, 2.0), S(1.0, 6.0, 0.0, 44.0));
    HI_ASSERT_SIMD_EQ(S(0.0, 2.0, 3.0, 42.0) - S(1.0, 4.0, -3.0, 2.0), S(-1.0, -2.0, 6.0, 40.0));
    HI_ASSERT_SIMD_EQ(S(0.0, 2.0, 3.0, 42.0) * S(1.0, 4.0, -3.0, 2.0), S(0.0, 8.0, -9.0, 84.0));
    HI_ASSERT_SIMD_EQ(S(0.0, 2.0, 3.0, 42.0) / S(1.0, 4.0, -3.0, 2.0), S(0.0, 0.5, -1.0, 21.0));
    HI_ASSERT_SIMD_EQ(min(S(0.0, 2.0, 0.0, 42.0), S(1.0, 0.0, -3.0, 1.0)), S(0.0, 0.0, -3.0, 1.0));
    HI_ASSERT_SIMD_EQ(max(S(0.0, 2.0, 0.0, 42.0), S(1.0, 0.0, -3.0, 1.0)), S(1.0, 2.0, 0.0, 42.0));
    HI_ASSERT_SIMD_EQ(abs(S(0.0, 2.2, -3.2, -3.6)), S(0.0, 2.2, 3.2, 3.6));
    // _mm_rcp_ps(): The maximum relative error for this approximation is less than 1.5*2^-12 = 0.0003662109375.
    ASSERT_TRUE(almost_equal(rcp(S(1.0, 2.0, 0.5, -4.0)), S(1.0, 0.5, 2.0, -0.25), 0.0005));
    HI_ASSERT_SIMD_EQ(sqrt(S(1.0, 1.5625, 4.0, 9.0)), S(1.0, 1.25, 2.0, 3.0));
    // _mm_rsqrt_ps(): The maximum relative error for this approximation is less than 1.5*2^-12 = 0.0003662109375.
    ASSERT_TRUE(almost_equal(rsqrt(S(1.0, 1.5625, 4.0, 9.0)), S(1.0, 0.8, 0.5, 0.3333333333333), 0.0005));

#ifdef HI_HAS_SSE3
    HI_ASSERT_SIMD_EQ(interleaved_sub_add(S(0.0, 2.0, 3.0, 42.0), S(1.0, 4.0, -3.0, 2.0)), S(-1.0, 6.0, 6.0, 44.0));
#endif

#ifdef HI_HAS_SSE4_1
    HI_ASSERT_SIMD_EQ(floor(S(0.0, 2.2, -3.2, -3.6)), S(0.0, 2.0, -4.0, -4.0));
    HI_ASSERT_SIMD_EQ(ceil(S(0.0, 2.2, -3.2, -3.6)), S(0.0, 3.0, -3.0, -3.0));
    HI_ASSERT_SIMD_EQ(round<>(S(0.0, 2.2, -3.2, -3.6)), S(0.0, 2.0, -3.0, -4.0));
#endif
}

TEST(native_f64x4, bit_wise)
{
    HI_ASSERT_SIMD_EQ(S(0.0, 2.0, 0.0, 42.0) | S(1.0, 0.0, -3.0, 0.0), S(1.0, 2.0, -3.0, 42.0));
    HI_ASSERT_SIMD_EQ(S(1.0, 2.0, 3.0, 42.0) & S::from_mask(0b1010), S(0.0, 2.0, 0.0, 42.0));
    HI_ASSERT_SIMD_EQ(S::from_mask(0b0011) ^ S::from_mask(0b1010), S::from_mask(0b1001));
    HI_ASSERT_SIMD_EQ(~S::from_mask(0b1010), S::from_mask(0b0101));

    HI_ASSERT_SIMD_EQ(not_and(S::from_mask(0b1010), S(1.0, 2.0, 3.0, 42.0)), S(1.0, 0.0, 3.0, 0.0));
}

TEST(native_f64x4, access)
{
    auto tmp = S(1.0, 2.0, 3.0, 4.0);

    ASSERT_EQ(get<0>(tmp), 1.0);
    ASSERT_EQ(get<1>(tmp), 2.0);
    ASSERT_EQ(get<2>(tmp), 3.0);
    ASSERT_EQ(get<3>(tmp), 4.0);

    HI_ASSERT_SIMD_EQ(insert<0>(tmp, 42.0), S(42.0, 2.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(insert<1>(tmp, 42.0), S(1.0, 42.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(insert<2>(tmp, 42.0), S(1.0, 2.0, 42.0, 4.0));
    HI_ASSERT_SIMD_EQ(insert<3>(tmp, 42.0), S(1.0, 2.0, 3.0, 42.0));

    HI_ASSERT_SIMD_EQ(set_zero<0b0000>(tmp), S(1.0, 2.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b0001>(tmp), S(0.0, 2.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b0010>(tmp), S(1.0, 0.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b0100>(tmp), S(1.0, 2.0, 0.0, 4.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b1000>(tmp), S(1.0, 2.0, 3.0, 0.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b1001>(tmp), S(0.0, 2.0, 3.0, 0.0));
    HI_ASSERT_SIMD_EQ(set_zero<0b1111>(tmp), S(0.0, 0.0, 0.0, 0.0));
}

TEST(native_f64x4, blend)
{
    auto a = S(1.0, 2.0, 3.0, 4.0);
    auto b = S(42.0, 43.0, 44.0, 45.0);

    HI_ASSERT_SIMD_EQ(blend<0b0000>(a, b), S(1.0, 2.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(blend<0b0001>(a, b), S(42.0, 2.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(blend<0b0010>(a, b), S(1.0, 43.0, 3.0, 4.0));
    HI_ASSERT_SIMD_EQ(blend<0b0100>(a, b), S(1.0, 2.0, 44.0, 4.0));
    HI_ASSERT_SIMD_EQ(blend<0b1000>(a, b), S(1.0, 2.0, 3.0, 45.0));
    HI_ASSERT_SIMD_EQ(blend<0b1001>(a, b), S(42.0, 2.0, 3.0, 45.0));
    HI_ASSERT_SIMD_EQ(blend<0b1111>(a, b), S(42.0, 43.0, 44.0, 45.0));
}

TEST(native_f64x4, permute)
{
    auto tmp = S(2.0, 3.0, 4.0, 5.0);

    HI_ASSERT_SIMD_EQ(permute<"abcd">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(permute<"xyzw">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(permute<"0000">(tmp), S(2.0, 3.0, 4.0, 5.0));

    HI_ASSERT_SIMD_EQ(permute<"dcba">(tmp), S(5.0, 4.0, 3.0, 2.0));
    HI_ASSERT_SIMD_EQ(permute<"wzyx">(tmp), S(5.0, 4.0, 3.0, 2.0));

    HI_ASSERT_SIMD_EQ(permute<"axcd">(tmp), S(2.0, 2.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(permute<"aycd">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(permute<"azcd">(tmp), S(2.0, 4.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(permute<"awcd">(tmp), S(2.0, 5.0, 4.0, 5.0));

    HI_ASSERT_SIMD_EQ(permute<"aaaa">(tmp), S(2.0, 2.0, 2.0, 2.0));
    HI_ASSERT_SIMD_EQ(permute<"xxxx">(tmp), S(2.0, 2.0, 2.0, 2.0));
    HI_ASSERT_SIMD_EQ(permute<"bbbb">(tmp), S(3.0, 3.0, 3.0, 3.0));
    HI_ASSERT_SIMD_EQ(permute<"cccc">(tmp), S(4.0, 4.0, 4.0, 4.0));
    HI_ASSERT_SIMD_EQ(permute<"dddd">(tmp), S(5.0, 5.0, 5.0, 5.0));
}

TEST(native_f64x4, swizzle)
{
    auto tmp = S(2.0, 3.0, 4.0, 5.0);

    HI_ASSERT_SIMD_EQ(swizzle<"abcd">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(swizzle<"xyzw">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(swizzle<"0000">(tmp), S(0.0, 0.0, 0.0, 0.0));

    HI_ASSERT_SIMD_EQ(swizzle<"dcba">(tmp), S(5.0, 4.0, 3.0, 2.0));
    HI_ASSERT_SIMD_EQ(swizzle<"wzyx">(tmp), S(5.0, 4.0, 3.0, 2.0));

    HI_ASSERT_SIMD_EQ(swizzle<"axcd">(tmp), S(2.0, 2.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(swizzle<"aycd">(tmp), S(2.0, 3.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(swizzle<"azcd">(tmp), S(2.0, 4.0, 4.0, 5.0));
    HI_ASSERT_SIMD_EQ(swizzle<"awcd">(tmp), S(2.0, 5.0, 4.0, 5.0));

    HI_ASSERT_SIMD_EQ(swizzle<"aaaa">(tmp), S(2.0, 2.0, 2.0, 2.0));
    HI_ASSERT_SIMD_EQ(swizzle<"xxxx">(tmp), S(2.0, 2.0, 2.0, 2.0));
    HI_ASSERT_SIMD_EQ(swizzle<"bbbb">(tmp), S(3.0, 3.0, 3.0, 3.0));
    HI_ASSERT_SIMD_EQ(swizzle<"cccc">(tmp), S(4.0, 4.0, 4.0, 4.0));
    HI_ASSERT_SIMD_EQ(swizzle<"dddd">(tmp), S(5.0, 5.0, 5.0, 5.0));

    HI_ASSERT_SIMD_EQ(swizzle<"0000">(tmp), S(0.0, 0.0, 0.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1000">(tmp), S(1.0, 0.0, 0.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"0100">(tmp), S(0.0, 1.0, 0.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"0010">(tmp), S(0.0, 0.0, 1.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"0001">(tmp), S(0.0, 0.0, 0.0, 1.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1001">(tmp), S(1.0, 0.0, 0.0, 1.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1111">(tmp), S(1.0, 1.0, 1.0, 1.0));

    HI_ASSERT_SIMD_EQ(swizzle<"00b0">(tmp), S(0.0, 0.0, 3.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1b00">(tmp), S(1.0, 3.0, 0.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"010b">(tmp), S(0.0, 1.0, 0.0, 3.0));
    HI_ASSERT_SIMD_EQ(swizzle<"0b10">(tmp), S(0.0, 3.0, 1.0, 0.0));
    HI_ASSERT_SIMD_EQ(swizzle<"b001">(tmp), S(3.0, 0.0, 0.0, 1.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1b01">(tmp), S(1.0, 3.0, 0.0, 1.0));
    HI_ASSERT_SIMD_EQ(swizzle<"11b1">(tmp), S(1.0, 1.0, 3.0, 1.0));
    HI_ASSERT_SIMD_EQ(swizzle<"1111">(tmp), S(1.0, 1.0, 1.0, 1.0));
}

TEST(native_f64x4, horizontal)
{
    HI_ASSERT_SIMD_EQ(horizontal_add(S(2.0, 3.0, 4.0, 5.0), S(12.0, 13.0, 14.0, 15.0)), S(5.0, 9.0, 25.0, 29.0));
    HI_ASSERT_SIMD_EQ(horizontal_sub(S(42.0, 3.0, 34.0, 5.0), S(2.0, 13.0, 24.0, 15.0)), S(39.0, 29.0, -11.0, 9.0));
    HI_ASSERT_SIMD_EQ(horizontal_sum(S(1.0, 2.0, 3.0, 4.0)), S::broadcast(10.0));
}

hi_warning_pop();
