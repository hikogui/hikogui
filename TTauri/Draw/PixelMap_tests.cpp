// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include "Path.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
using namespace TTauri::Draw;

TEST(PixelMapTests, renderMaskFromPath) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();

    auto path = Path();
    path.moveTo({1, 1});
    path.lineTo({2, 1});
    path.lineTo({2, 2});
    path.lineTo({1, 2});
    path.close();

    path.render(mask);
    ASSERT_EQ(mask[0][0], 0);
    ASSERT_EQ(mask[0][1], 0);
    ASSERT_EQ(mask[0][2], 0);
    ASSERT_EQ(mask[0][3], 0);
    ASSERT_EQ(mask[0][4], 0);
    ASSERT_EQ(mask[0][5], 0);
    ASSERT_EQ(mask[0][6], 0);
    ASSERT_EQ(mask[0][7], 0);
    ASSERT_EQ(mask[0][8], 0);
    ASSERT_EQ(mask[1][0], 0);
    ASSERT_EQ(mask[1][1], 0);
    ASSERT_EQ(mask[1][2], 0);
    ASSERT_EQ(mask[1][3], 255);
    ASSERT_EQ(mask[1][4], 255);
    ASSERT_EQ(mask[1][5], 255);
    ASSERT_EQ(mask[1][6], 0);
    ASSERT_EQ(mask[1][7], 0);
    ASSERT_EQ(mask[1][8], 0);
    ASSERT_EQ(mask[2][0], 0);
    ASSERT_EQ(mask[2][1], 0);
    ASSERT_EQ(mask[2][2], 0);
    ASSERT_EQ(mask[2][3], 0);
    ASSERT_EQ(mask[2][4], 0);
    ASSERT_EQ(mask[2][5], 0);
    ASSERT_EQ(mask[2][6], 0);
    ASSERT_EQ(mask[2][7], 0);
    ASSERT_EQ(mask[2][8], 0);
}

TEST(PixelMapTests, maskComposit) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();
    mask[1][3] = 255;
    mask[1][4] = 255;
    mask[1][5] = 255;

    auto image = PixelMap<uint64_t>(3, 3);
    image.clear();

    composit(image, 0xffffffffffffffffULL, mask);

    ASSERT_EQ(image[0][0], 0);
    ASSERT_EQ(image[0][1], 0);
    ASSERT_EQ(image[0][2], 0);
    ASSERT_EQ(image[1][0], 0);
    ASSERT_EQ(image[1][1], 0xffffffffffffffffULL);
    ASSERT_EQ(image[1][2], 0);
    ASSERT_EQ(image[2][0], 0);
    ASSERT_EQ(image[2][1], 0);
    ASSERT_EQ(image[2][2], 0);
}

TEST(PixelMapTests, maskComposit2) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();
    mask[1][3] = 255;
    mask[1][4] = 255;
    mask[1][5] = 255;

    auto image = PixelMap<uint64_t>(3, 3);
    image.clear();

    composit(image, 0x1234'5678'9abc'ffffULL, mask);

    ASSERT_EQ(image[1][1], 0x1234'5678'9abc'ffffULL);
}

TEST(PixelMapTests, maskComposit3) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();
    mask[1][3] = 0x88;
    mask[1][4] = 0x44;
    mask[1][5] = 0x22;

    auto image = PixelMap<uint64_t>(3, 3);
    image.clear();

    composit(image, 0xffff'ffff'ffff'ffffULL, mask);

    ASSERT_EQ(image[1][1], 0x8888'4444'2222'4f4fULL);
}

