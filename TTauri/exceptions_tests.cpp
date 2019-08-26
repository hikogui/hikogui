// Copyright 2019 Pokitec
// All rights reserved.

#include "exceptions.hpp"
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
        ASSERT_EQ(e.name(), "key-error"s);

        let optional_line = e.get<"line"_tag,int>();
        ASSERT_EQ(e.source_line, 17);

        let optional_key = e.get<"key"_tag,std::string>();
        ASSERT_EQ(*optional_key, "foo"s);

        ASSERT_EQ(e.error_info_string(), "(key: \"foo\")"s);
    }

    ASSERT_EQ(read_counter<key_error::TAG>(), current_count + 1);
}
