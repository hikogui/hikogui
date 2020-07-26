// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/encoding/SHA2.hpp"
#include "ttauri/required.hpp"
#include <gtest/gtest.h>
#include <iostream>

using namespace std;
using namespace tt;

TEST(SHA2, EmptySHA224) {

    auto hash = SHA224();
    hash.add(std::string(""));
    auto bytes = hash.get_bytes();
}
