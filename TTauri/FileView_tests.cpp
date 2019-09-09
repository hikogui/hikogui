// Copyright 2019 Pokitec
// All rights reserved.

#include "FileView.hpp"
#include "required.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(FileView, ViewFileForRead) {
    let view = FileView(URL("file:TestFiles/file_view.txt"));

    let *test = reinterpret_cast<char const *>(view.bytes().data());
    ASSERT_TRUE(strncmp(test, "The quick brown", 15) == 0);
}