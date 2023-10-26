// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URI.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace hi;

TEST(URI, percent_decode)
{
    ASSERT_EQ(URI::decode("Program%20Files"), "Program Files");
}

TEST(URI, scheme_only)
{
    hilet u = URI("file:");
    ASSERT_EQ(u.scheme(), "file");
}

TEST(URI, file_absolute_dir)
{
    hilet u = URI("file:///C:/Program%20Files/RenderDoc/");
    ASSERT_EQ(u.scheme(), "file");
    ASSERT_TRUE(u.authority());
    ASSERT_EQ(u.authority()->host(), "");
    auto path = u.path();
    ASSERT_TRUE(path.absolute());
    ASSERT_EQ(path.size(), 5);
    ASSERT_EQ(path[0], "");
    ASSERT_EQ(path[1], "C:");
    ASSERT_EQ(path[2], "Program Files");
    ASSERT_EQ(path[3], "RenderDoc");
    ASSERT_EQ(path[4], "");
}

TEST(URI, file_absolute_dir_file)
{
    hilet u = URI("file:///C:/Program%20Files/RenderDoc/renderdoc.dll");
    ASSERT_EQ(u.scheme(), "file");
    ASSERT_TRUE(u.authority());
    ASSERT_EQ(u.authority()->host(), "");
    auto path = u.path();
    ASSERT_TRUE(path.absolute());
    ASSERT_EQ(path.size(), 5);
    ASSERT_EQ(path[0], "");
    ASSERT_EQ(path[1], "C:");
    ASSERT_EQ(path[2], "Program Files");
    ASSERT_EQ(path[3], "RenderDoc");
    ASSERT_EQ(path[4], "renderdoc.dll");
}

TEST(URI, file_relative_dir)
{
    hilet u = URI("file:C:/Program%20Files/RenderDoc/");
    ASSERT_EQ(u.scheme(), "file");
    ASSERT_FALSE(u.authority());
    auto path = u.path();
    ASSERT_FALSE(path.absolute());
    ASSERT_EQ(path.size(), 4);
    ASSERT_EQ(path[0], "C:");
    ASSERT_EQ(path[1], "Program Files");
    ASSERT_EQ(path[2], "RenderDoc");
    ASSERT_EQ(path[3], "");
}

TEST(URI, file_relative_dir_file)
{
    hilet u = URI("file:C:/Program%20Files/RenderDoc/renderdoc.dll");
    ASSERT_EQ(u.scheme(), "file");
    ASSERT_FALSE(u.authority());
    auto path = u.path();
    ASSERT_FALSE(path.absolute());
    ASSERT_EQ(path.size(), 4);
    ASSERT_EQ(path[0], "C:");
    ASSERT_EQ(path[1], "Program Files");
    ASSERT_EQ(path[2], "RenderDoc");
    ASSERT_EQ(path[3], "renderdoc.dll");
}

TEST(URI, reference_resolution_normal)
{
    // RFC-3986 Chapter 5.4.1.
    hilet base = URI("http://a/b/c/d;p?q");

    ASSERT_EQ(base / "g:h", "g:h");
    ASSERT_EQ(base / "g", "http://a/b/c/g");
    ASSERT_EQ(base / "./g", "http://a/b/c/g");
    ASSERT_EQ(base / "g/", "http://a/b/c/g/");
    ASSERT_EQ(base / "/g", "http://a/g");
    ASSERT_EQ(base / "//g", "http://g");
    ASSERT_EQ(base / "?y", "http://a/b/c/d;p?y");
    ASSERT_EQ(base / "g?y", "http://a/b/c/g?y");
    ASSERT_EQ(base / "#s", "http://a/b/c/d;p?q#s");
    ASSERT_EQ(base / "g#s", "http://a/b/c/g#s");
    ASSERT_EQ(base / "g?y#s", "http://a/b/c/g?y#s");
    ASSERT_EQ(base / ";x", "http://a/b/c/;x");
    ASSERT_EQ(base / "g;x", "http://a/b/c/g;x");
    ASSERT_EQ(base / "g;x?y#s", "http://a/b/c/g;x?y#s");
    ASSERT_EQ(base / "", "http://a/b/c/d;p?q");
    ASSERT_EQ(base / ".", "http://a/b/c/");
    ASSERT_EQ(base / "./", "http://a/b/c/");
    ASSERT_EQ(base / "..", "http://a/b/");
    ASSERT_EQ(base / "../", "http://a/b/");
    ASSERT_EQ(base / "../g", "http://a/b/g");
    ASSERT_EQ(base / "../..", "http://a/");
    ASSERT_EQ(base / "../../", "http://a/");
    ASSERT_EQ(base / "../../g", "http://a/g");
}

TEST(URI, reference_resolution_abnormal)
{
    // RFC-3986 Chapter 5.4.2.
    hilet base = URI("http://a/b/c/d;p?q");

    ASSERT_EQ(base / "../../../g", "http://a/g");
    ASSERT_EQ(base / "../../../../g", "http://a/g");

    ASSERT_EQ(base / "/./g", "http://a/g");
    ASSERT_EQ(base / "/../g", "http://a/g");
    ASSERT_EQ(base / "g.", "http://a/b/c/g.");
    ASSERT_EQ(base / ".g", "http://a/b/c/.g");
    ASSERT_EQ(base / "g..", "http://a/b/c/g..");
    ASSERT_EQ(base / "..g", "http://a/b/c/..g");

    ASSERT_EQ(base / "./../g", "http://a/b/g");
    ASSERT_EQ(base / "./g/.", "http://a/b/c/g/");
    ASSERT_EQ(base / "g/./h", "http://a/b/c/g/h");
    ASSERT_EQ(base / "g/../h", "http://a/b/c/h");
    ASSERT_EQ(base / "g;x=1/./y", "http://a/b/c/g;x=1/y");
    ASSERT_EQ(base / "g;x=1/../y", "http://a/b/c/y");

    ASSERT_EQ(base / "g?y/./x", "http://a/b/c/g?y/./x");
    ASSERT_EQ(base / "g?y/../x", "http://a/b/c/g?y/../x");
    ASSERT_EQ(base / "g#s/./x", "http://a/b/c/g#s/./x");
    ASSERT_EQ(base / "g#s/../x", "http://a/b/c/g#s/../x");

    // Strict.
    ASSERT_EQ(base / "http:g", "http:g");
}
