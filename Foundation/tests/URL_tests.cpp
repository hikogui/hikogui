// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/URL.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(URLTests, parsing) {
    let a = URL("scheme://user:password@hostname:1234/path1/path2?query#fragment");

    ASSERT_EQ(a.scheme(), "scheme");
    ASSERT_EQ(a.isAbsolute(), true);
    ASSERT_EQ(a.pathSegments().at(0), "path1");
    ASSERT_EQ(a.pathSegments().at(1), "path2");
    ASSERT_EQ(a.query(), "query");
    ASSERT_EQ(a.fragment(), "fragment");
}

TEST(URLTests, relativePath) {
    let a = URL("file:foo/bar.txt");

    ASSERT_EQ(a.path(), "foo/bar.txt");
}
