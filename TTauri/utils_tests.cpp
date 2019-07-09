// Copyright 2019 Pokitec
// All rights reserved.

#include <TTauri/utils.hpp>
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(UtilsTests, split) {
    let result = split(std::string{"path1/path2"}, '/');
    let check_value = std::vector<std::string>{"path1", "path2"};
    ASSERT_EQ(result, check_value);
}
