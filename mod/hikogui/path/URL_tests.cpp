// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "URL.hpp"
#include "glob.hpp"
#include "../algorithm/algorithm.hpp"
#include "../macros.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace hi;

TEST(URL, parsing)
{
    hilet a = URL("scheme://user:password@hostname:1234/path1/path2?query#fragment");

    ASSERT_EQ(a.scheme(), "scheme");
    ASSERT_EQ(a.path().absolute(), true);
    ASSERT_EQ(a.path().at(0), "");
    ASSERT_EQ(a.path().at(1), "path1");
    ASSERT_EQ(a.path().at(2), "path2");
    ASSERT_EQ(a.query(), "query");
    ASSERT_EQ(a.fragment(), "fragment");
}

TEST(URL, relativePath)
{
    hilet a = URL("file:foo/bar.txt");

    ASSERT_EQ(a.filesystem_path(), "foo/bar.txt");
}

TEST(URL, glob1)
{
    auto txt_files = make_vector(glob(library_source_dir() / "tests" / "data" / "*.txt"));

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) {
        return x.filename() == "file_view.txt";
    }));
    ASSERT_FALSE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) {
        return x.filename() == "HikoGUI_Foundation.lib";
    }));
}

TEST(URL, glob2)
{
    auto txt_files = make_vector(glob(library_source_dir() / "tests" / "data" / "**" / "*.txt"));

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) {
        return x.filename() == "glob2.txt";
    }));
}
