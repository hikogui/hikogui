// Copyright Take Vos 2019.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URI.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(URI) {

TEST_CASE(percent_decode)
{
    REQUIRE(hi::URI::decode("Program%20Files") == "Program Files");
}

TEST_CASE(scheme_only)
{
    auto const u = hi::URI("file:");
    REQUIRE(u.scheme() == "file");
}

TEST_CASE(file_absolute_dir)
{
    auto const u = hi::URI("file:///C:/Program%20Files/RenderDoc/");
    REQUIRE(u.scheme() == "file");
    REQUIRE(static_cast<bool>(u.authority()));
    REQUIRE(u.authority()->host() == "");
    auto path = u.path();
    REQUIRE(path.absolute());
    REQUIRE(path.size() == 5);
    REQUIRE(path[0] == "");
    REQUIRE(path[1] == "C:");
    REQUIRE(path[2] == "Program Files");
    REQUIRE(path[3] == "RenderDoc");
    REQUIRE(path[4] == "");
}

TEST_CASE(file_absolute_dir_file)
{
    auto const u = hi::URI("file:///C:/Program%20Files/RenderDoc/renderdoc.dll");
    REQUIRE(u.scheme() == "file");
    REQUIRE(static_cast<bool>(u.authority()));
    REQUIRE(u.authority()->host() == "");
    auto path = u.path();
    REQUIRE(path.absolute());
    REQUIRE(path.size() == 5);
    REQUIRE(path[0] == "");
    REQUIRE(path[1] == "C:");
    REQUIRE(path[2] == "Program Files");
    REQUIRE(path[3] == "RenderDoc");
    REQUIRE(path[4] == "renderdoc.dll");
}

TEST_CASE(file_relative_dir)
{
    auto const u = hi::URI("file:C:/Program%20Files/RenderDoc/");
    REQUIRE(u.scheme() == "file");
    REQUIRE(not u.authority());
    auto path = u.path();
    REQUIRE(not path.absolute());
    REQUIRE(path.size() == 4);
    REQUIRE(path[0] == "C:");
    REQUIRE(path[1] == "Program Files");
    REQUIRE(path[2] == "RenderDoc");
    REQUIRE(path[3] == "");
}

TEST_CASE(file_relative_dir_file)
{
    auto const u = hi::URI("file:C:/Program%20Files/RenderDoc/renderdoc.dll");
    REQUIRE(u.scheme() == "file");
    REQUIRE(not u.authority());
    auto path = u.path();
    REQUIRE(not path.absolute());
    REQUIRE(path.size() == 4);
    REQUIRE(path[0] == "C:");
    REQUIRE(path[1] == "Program Files");
    REQUIRE(path[2] == "RenderDoc");
    REQUIRE(path[3] == "renderdoc.dll");
}

TEST_CASE(reference_resolution_normal)
{
    // RFC-3986 Chapter 5.4.1.
    auto const base = hi::URI("http://a/b/c/d;p?q");

    REQUIRE(base / "g:h" == "g:h");
    REQUIRE(base / "g" == "http://a/b/c/g");
    REQUIRE(base / "./g" == "http://a/b/c/g");
    REQUIRE(base / "g/" == "http://a/b/c/g/");
    REQUIRE(base / "/g" == "http://a/g");
    REQUIRE(base / "//g" == "http://g");
    REQUIRE(base / "?y" == "http://a/b/c/d;p?y");
    REQUIRE(base / "g?y" == "http://a/b/c/g?y");
    REQUIRE(base / "#s" == "http://a/b/c/d;p?q#s");
    REQUIRE(base / "g#s" == "http://a/b/c/g#s");
    REQUIRE(base / "g?y#s" == "http://a/b/c/g?y#s");
    REQUIRE(base / ";x" == "http://a/b/c/;x");
    REQUIRE(base / "g;x" == "http://a/b/c/g;x");
    REQUIRE(base / "g;x?y#s" == "http://a/b/c/g;x?y#s");
    REQUIRE(base / "" == "http://a/b/c/d;p?q");
    REQUIRE(base / "." == "http://a/b/c/");
    REQUIRE(base / "./" == "http://a/b/c/");
    REQUIRE(base / ".." == "http://a/b/");
    REQUIRE(base / "../" == "http://a/b/");
    REQUIRE(base / "../g" == "http://a/b/g");
    REQUIRE(base / "../.." == "http://a/");
    REQUIRE(base / "../../" == "http://a/");
    REQUIRE(base / "../../g" == "http://a/g");
}

TEST_CASE(reference_resolution_abnormal)
{
    // RFC-3986 Chapter 5.4.2.
    auto const base = hi::URI("http://a/b/c/d;p?q");

    REQUIRE(base / "../../../g" == "http://a/g");
    REQUIRE(base / "../../../../g" == "http://a/g");

    REQUIRE(base / "/./g" == "http://a/g");
    REQUIRE(base / "/../g" == "http://a/g");
    REQUIRE(base / "g." == "http://a/b/c/g.");
    REQUIRE(base / ".g" == "http://a/b/c/.g");
    REQUIRE(base / "g.." == "http://a/b/c/g..");
    REQUIRE(base / "..g" == "http://a/b/c/..g");

    REQUIRE(base / "./../g" == "http://a/b/g");
    REQUIRE(base / "./g/." == "http://a/b/c/g/");
    REQUIRE(base / "g/./h" == "http://a/b/c/g/h");
    REQUIRE(base / "g/../h" == "http://a/b/c/h");
    REQUIRE(base / "g;x=1/./y" == "http://a/b/c/g;x=1/y");
    REQUIRE(base / "g;x=1/../y" == "http://a/b/c/y");

    REQUIRE(base / "g?y/./x" == "http://a/b/c/g?y/./x");
    REQUIRE(base / "g?y/../x" == "http://a/b/c/g?y/../x");
    REQUIRE(base / "g#s/./x" == "http://a/b/c/g#s/./x");
    REQUIRE(base / "g#s/../x" == "http://a/b/c/g#s/../x");

    // Strict.
    REQUIRE(base / "http:g" == "http:g");
}

};
