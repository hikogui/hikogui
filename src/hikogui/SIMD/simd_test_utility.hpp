// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "simd.hpp"
#include <gtest/gtest.h>

namespace hi { inline namespace v1 { namespace detail {

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

}}} // namespace hi::v1::detail

#define HI_ASSERT_SIMD_EQ(val1, val2) ASSERT_PRED_FORMAT2(hi::detail::CmpHelperEQ, val1, val2)
#define HI_ASSERT_SIMD_NE(val1, val2) ASSERT_PRED_FORMAT2(hi::detail::CmpHelperNE, val1, val2)
