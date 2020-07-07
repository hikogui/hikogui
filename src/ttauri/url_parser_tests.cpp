// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/required.hpp"
#include "ttauri/url_parser.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(urlParserTests, parsing) {
    ttlet parts = parse_url("file:");
    ASSERT_FALSE(parts.absolute);
    ASSERT_EQ(parts.segments.size(), 0);
}

TEST(urlParserTests, filenameWithDriveLetter) {
    ttlet parts = parse_url("file:///C:/Program%20Files/RenderDoc/renderdoc.dll");
    ASSERT_TRUE(parts.absolute);
    ASSERT_EQ(parts.scheme, "file");
    ASSERT_EQ(parts.authority, "");
    ASSERT_EQ(parts.drive, "C");
    ASSERT_EQ(ssize(parts.segments), 3);
    ASSERT_EQ(parts.segments[0], "Program%20Files");
    ASSERT_EQ(parts.segments[1], "RenderDoc");
    ASSERT_EQ(parts.segments[2], "renderdoc.dll");
}
