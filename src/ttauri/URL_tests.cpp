// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/URL.hpp"
#include "ttauri/algorithm.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::literals;
using namespace tt;

TEST(URLTests, parsing) {
    ttlet a = URL("scheme://user:password@hostname:1234/path1/path2?query#fragment");

    ASSERT_EQ(a.scheme(), "scheme");
    ASSERT_EQ(a.isAbsolute(), true);
    ASSERT_EQ(a.pathSegments().at(0), "path1");
    ASSERT_EQ(a.pathSegments().at(1), "path2");
    ASSERT_EQ(a.query(), "query");
    ASSERT_EQ(a.fragment(), "fragment");
}

TEST(URLTests, relativePath) {
    ttlet a = URL("file:foo/bar.txt");

    ASSERT_EQ(a.path(), "foo/bar.txt");
}

TEST(URLTests, glob1) {
    ttlet executableDirectory = URL::urlFromExecutableDirectory();
    
    ttlet txt_file_glob = executableDirectory.urlByAppendingPath("*.txt");
    auto txt_files = txt_file_glob.urlsByScanningWithGlobPattern();

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return x.path().ends_with("file_view.txt"s); }));
    ASSERT_FALSE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return x.path().ends_with("TTauri_Foundation.lib"s); }));
}

TEST(URLTests, glob2) {
    ttlet executableDirectory = URL::urlFromExecutableDirectory();

    ttlet txt_file_glob = executableDirectory.urlByAppendingPath("**/*.txt");
    auto txt_files = txt_file_glob.urlsByScanningWithGlobPattern();

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return x.path().ends_with("glob2.txt"s); }));
}
