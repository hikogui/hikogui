// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/URI.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(URI, scheme_only)
{
    hilet u = URI("file:");
    ASSERT_EQ(u.scheme(), "file");
}

TEST(URI, file_absolute_path)
{
    hilet u = URI("file:///C:/Program%20Files/RenderDoc/renderdoc.dll");
    ASSERT_EQ(u.scheme(), "file");
    ASSERT_EQ(u.host(), "");
    ASSERT_TRUE(u.path_is_absolute());
    ASSERT_EQ(u.size(), 4);
    ASSERT_EQ(u[0], "C:");
    ASSERT_EQ(u[1], "Program Files");
    ASSERT_EQ(u[2], "RenderDoc");
    ASSERT_EQ(u[3], "renderdoc.dll");
}
