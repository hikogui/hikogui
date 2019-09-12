// Copyright 2019 Pokitec
// All rights reserved.

#include "string_tag.hpp"
#include "TTauri/Required/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(StringTag, CompileTime) {
    ASSERT_EQ("foo"_tag, "foo"_tag);
    ASSERT_EQ("bar"_tag, "bar"_tag);
    ASSERT_EQ("baz"_tag, "baz"_tag);

    ASSERT_NE("foo"_tag, "bar"_tag);
    ASSERT_NE("foo"_tag, "baz"_tag);
    ASSERT_NE("bar"_tag, "foo"_tag);
    ASSERT_NE("bar"_tag, "baz"_tag);
    ASSERT_NE("baz"_tag, "foo"_tag);
    ASSERT_NE("baz"_tag, "bar"_tag);
}