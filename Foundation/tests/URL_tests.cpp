// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/algorithm.hpp"
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

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return ends_with(x.path(), "file_view.txt"s); }));
    ASSERT_FALSE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return ends_with(x.path(), "TTauri_Foundation.lib"s); }));
}

TEST(URLTests, glob2) {
    ttlet executableDirectory = URL::urlFromExecutableDirectory();

    ttlet txt_file_glob = executableDirectory.urlByAppendingPath("**/*.hpp");
    auto txt_files = txt_file_glob.urlsByScanningWithGlobPattern();

    ASSERT_TRUE(std::any_of(txt_files.begin(), txt_files.end(), [](auto x) { return ends_with(x.path(), "include/TTauri/Foundation/config.hpp"s); }));
}
