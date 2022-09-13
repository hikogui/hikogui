// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/utility.hpp"
#include "hikogui/url_parser.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(urlParserTests, parsing)
{
    hilet parts = parse_url("file:");
    ASSERT_FALSE(parts.absolute);
    ASSERT_EQ(parts.segments.size(), 0);
}

TEST(urlParserTests, filenameWithDriveLetter)
{
    hilet parts = parse_url("file:///C:/Program%20Files/RenderDoc/renderdoc.dll");
    ASSERT_TRUE(parts.absolute);
    ASSERT_EQ(parts.scheme, "file");
    ASSERT_EQ(parts.authority, "");
    ASSERT_EQ(parts.drive, "C");
    ASSERT_EQ(ssize(parts.segments), 3);
    ASSERT_EQ(parts.segments[0], "Program%20Files");
    ASSERT_EQ(parts.segments[1], "RenderDoc");
    ASSERT_EQ(parts.segments[2], "renderdoc.dll");
}
