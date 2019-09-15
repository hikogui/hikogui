// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/FileView.hpp"
#include "TTauri/Required/required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(FileView, ViewFileForRead) {
    let view = FileView(URL("file:file_view.txt"));

    let *test = reinterpret_cast<char const *>(view.bytes().data());
    ASSERT_TRUE(strncmp(test, "The quick brown", 15) == 0);
}
