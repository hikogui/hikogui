// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <gtest/gtest.h>

#define STATIC_ASSERT_TRUE(...) \
    static_assert(__VA_ARGS__, "unit test failed"); \
    ASSERT_TRUE(__VA_ARGS__)
    
#define STATIC_ASSERT_FALSE(...) static_assert(not (__VA_ARGS__), "unit test failed"); \
    ASSERT_FALSE(__VA_ARGS__)
