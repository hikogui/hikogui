// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "xorshift128p.hpp"
#include "../macros.hpp"
#include <iostream>
#include <string>
#include <limits>
#include <gtest/gtest.h>

using namespace std;
using namespace hi;

namespace xorshift128p_tests {

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

} // namespace xorshift128p_tests

#define HI_ASSERT_SIMD_EQ(val1, val2) ASSERT_PRED_FORMAT2(xorshift128p_tests::CmpHelperEQ, val1, val2)
#define HI_ASSERT_SIMD_NE(val1, val2) ASSERT_PRED_FORMAT2(xorshift128p_tests::CmpHelperNE, val1, val2)

TEST(xorshift128p, compare_64_and_128_bits)
{
    auto r1 = hi::xorshift128p();
    // Make a copy with the same seed.
    auto r2 = r1;

    for (auto i = 0; i != 100000; ++i) {
        auto expected = u64x2{};
        expected[0] = r1.next<uint64_t>();
        expected[1] = r1.next<uint64_t>();

        auto result = r2.next<u64x2>();
        HI_ASSERT_SIMD_EQ(result, expected);
    }
}
