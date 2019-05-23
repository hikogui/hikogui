// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
using namespace TTauri::Draw;

TEST(PixelMapTests, renderMaskFromPath) {
    auto mask = PixelMap<uint32_t>(3, 3);
    mask.clear();

    auto path = Path();
    path.moveTo({1, 1});
    path.lineTo({2, 1});
    path.lineTo({2, 2});
    path.lineTo({1, 2});
    path.close();

    renderMask(mask, path);
    ASSERT_EQ(mask[0][0], 0);
    ASSERT_EQ(mask[0][1], 0);
    ASSERT_EQ(mask[0][2], 0);
    ASSERT_EQ(mask[1][0], 0);
    ASSERT_EQ(mask[1][1], (1000 << 20) | (1000 << 10) | 1000);
    ASSERT_EQ(mask[1][2], 0);
    ASSERT_EQ(mask[2][0], 0);
    ASSERT_EQ(mask[2][1], 0);
    ASSERT_EQ(mask[2][2], 0);
}

TEST(PixelMapTests, maskComposit) {
    auto mask = PixelMap<uint32_t>(3, 3);
    mask.clear();
    mask[1][1] = (1000 << 20) | (1000 << 10) | 1000;

    auto image = PixelMap<uint32_t>(3, 3);
    image.clear();

    maskComposit(image, Color_sRGB(0xffffffff), mask);

    ASSERT_EQ(image[0][0], 0);
    ASSERT_EQ(image[0][1], 0);
    ASSERT_EQ(image[0][2], 0);
    ASSERT_EQ(image[1][0], 0);
    ASSERT_EQ(image[1][1], 0xffffffff);
    ASSERT_EQ(image[1][2], 0);
    ASSERT_EQ(image[2][0], 0);
    ASSERT_EQ(image[2][1], 0);
    ASSERT_EQ(image[2][2], 0);
}