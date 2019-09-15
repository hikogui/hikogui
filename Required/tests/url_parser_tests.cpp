// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/required.hpp"
#include "TTauri/Required/url_parser.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(urlParserTests, parsing) {
    let parts = parse_url("file:");
    ASSERT_FALSE(parts.absolute);
    ASSERT_EQ(parts.segments.size(), 0);
}
