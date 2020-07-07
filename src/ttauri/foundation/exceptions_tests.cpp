// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/exceptions.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace tt;

TEST(Exceptions, Default) {
    ttlet current_count = read_counter<key_error::TAG>();

    try {
        TTAURI_THROW(key_error("This is a key error").set<key_tag>("foo"s));
    } catch (error const &e) {
        ASSERT_TRUE(e.name().find("key_error") != std::string::npos);

        ttlet key = e.get<key_tag>();
        ASSERT_EQ(key, "foo"s);

        ASSERT_TRUE(e.error_info_string().find("key") != std::string::npos);
        ASSERT_TRUE(e.error_info_string().find("=\"foo\"") != std::string::npos);
    }

    ASSERT_EQ(read_counter<key_error::TAG>(), current_count + 1);
}
