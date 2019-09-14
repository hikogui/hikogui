// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/wsRGBA.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(wsRGBA, initialize) {
    ASSERT_EQ(wsRGBA{ 0x123456ff }.to_sRGBA_u32(), 0x123456ff);
    ASSERT_EQ(wsRGBA{ 0x6889abcd }.to_sRGBA_u32(), 0x6889abcd);
    ASSERT_EQ(wsRGBA{ 0x51c12bff }.to_sRGBA_u32(), 0x51c12bff);
}
