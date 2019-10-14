// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/exceptions.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace TTauri;

TEST(Exceptions, Default) {
    let current_count = read_counter<key_error::TAG>();

    try {
        TTAURI_THROW(key_error("This is a key error").set<"key"_tag>("foo"s));
    } catch (error const &e) {
        ASSERT_EQ(e.name(), "key_error"s);

        let key = e.get<"key"_tag>();
        ASSERT_EQ(key, "foo"s);

        ASSERT_EQ(e.error_info_string(), "key=\"foo\""s);
    }

    ASSERT_EQ(read_counter<key_error::TAG>(), current_count + 1);
}
