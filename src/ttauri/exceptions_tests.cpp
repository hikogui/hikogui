// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/exception.hpp"
#include "ttauri/error_info.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace tt;

TEST(error_info, default) {
    try {
        tt_error_info().set<key_tag>("foo"s);
        throw key_error("This is a key error");

    } catch (...) {
        ttlet key = error_info::pop<key_tag>();
        ASSERT_TRUE(key);
        ASSERT_EQ(*key, "foo"s);
    }
}
