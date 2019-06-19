// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include "SubpixelMask.hpp"
#include "Path.hpp"
#include "TTauri/Color.hpp"
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

    path.fill(mask);
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

    auto image = PixelMap<wsRGBApm>(3, 3);
    image.clear();

    let transparent = wsRGBApm{ 0.0, 0.0, 0.0, 0.0 };
    let white = wsRGBApm{ 1.0, 1.0, 1.0, 1.0 };
    composit(image, white, mask);

    ASSERT_EQ(image[0][0], transparent);
    ASSERT_EQ(image[0][1], transparent);
    ASSERT_EQ(image[0][2], transparent);
    ASSERT_EQ(image[1][0], transparent);
    ASSERT_EQ(image[1][1], white);
    ASSERT_EQ(image[1][2], transparent);
    ASSERT_EQ(image[2][0], transparent);
    ASSERT_EQ(image[2][1], transparent);
    ASSERT_EQ(image[2][2], transparent);
}

TEST(PixelMapTests, maskComposit2) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();
    mask[1][3] = 255;
    mask[1][4] = 255;
    mask[1][5] = 255;

    auto image = PixelMap<wsRGBApm>(3, 3);
    image.clear();

    let color = wsRGBApm{ 0.25, 0.50, 0.75, 1.0 };
    composit(image, color, mask);

    ASSERT_EQ(image[1][1], color);
}

TEST(PixelMapTests, maskComposit3) {
    auto mask = SubpixelMask(9, 3);
    mask.clear();
    mask[1][3] = 0x88;
    mask[1][4] = 0x44;
    mask[1][5] = 0x22;

    auto image = PixelMap<wsRGBApm>(3, 3);
    image.clear();

    let white = wsRGBApm{ 1.0, 1.0, 1.0, 1.0 };
    composit(image, white, mask);

    let alpha = ((0x88 + 0x44 + 0x22) / 3) / 255.0f;
    let red = 0x88 / 255.0f;
    let green = 0x44 / 255.0f;
    let blue = 0x22 / 255.0f;

    let compositColor = wsRGBApm{ red / alpha, green / alpha, blue / alpha, alpha };
    ASSERT_EQ(image[1][1].string(), compositColor.string());
}

