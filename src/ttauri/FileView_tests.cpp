// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/FileView.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(FileView, ViewFileForRead) {
    ttlet view = FileView(URL("file:file_view.txt"));

    ttlet *test = reinterpret_cast<char const *>(view.bytes().data());
    ASSERT_TRUE(strncmp(test, "The quick brown", 15) == 0);
}
