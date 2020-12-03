// Copyright 2019 Pokitec
// All rights reserved.

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
        ttlet key = error_info::get<key_tag>();
        ASSERT_TRUE(key);
        ASSERT_EQ(*key, "foo"s);
    }
}
