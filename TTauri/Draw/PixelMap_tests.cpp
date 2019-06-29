// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.inl"
#include "Path.hpp"
#include "BezierCurve.hpp"
#include "TTauri/Color.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;
using namespace TTauri::Draw;

TEST(PixelMapTests, rotate90) {
    auto mask = PixelMap<uint8_t>(2, 2);
    mask[1][0] = 3; mask[1][1] = 4;
    mask[0][0] = 1; mask[0][1] = 2;

    auto r = PixelMap<uint8_t>(2, 2);
    rotate90(r, mask);
    ASSERT_EQ(r[1][0], 4); ASSERT_EQ(r[1][1], 2);
    ASSERT_EQ(r[0][0], 3); ASSERT_EQ(r[0][1], 1);
}

TEST(PixelMapTests, rotate270) {
    auto mask = PixelMap<uint8_t>(2, 2);
    mask[1][0] = 3; mask[1][1] = 4;
    mask[0][0] = 1; mask[0][1] = 2;

    auto r = PixelMap<uint8_t>(2, 2);
    rotate270(r, mask);
    ASSERT_EQ(r[1][0], 1); ASSERT_EQ(r[1][1], 3);
    ASSERT_EQ(r[0][0], 2); ASSERT_EQ(r[0][1], 4);
}

TEST(PixelMapTests, renderMaskFromPath) {
    auto mask = PixelMap<uint8_t>(9, 3);
    fill(mask);

    auto path = Path();
    path.moveTo({1, 1});
    path.lineTo({2, 1});
    path.lineTo({2, 2});
    path.lineTo({1, 2});
    path.closeContour();

    auto beziers = path.getBeziers();
    for (auto &&bezier: beziers) {
        bezier *= glm::vec2{3.0, 1.0};
    }

    fill(mask, beziers);
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
    auto mask = PixelMap<uint8_t>(9, 3);
    fill(mask);
    mask[1][3] = 255;
    mask[1][4] = 255;
    mask[1][5] = 255;

    auto image = PixelMap<wsRGBA>(3, 3);
    fill(image);

    let transparent = wsRGBA{ 0.0, 0.0, 0.0, 0.0 };
    let white = wsRGBA{ 1.0, 1.0, 1.0, 1.0 };
    subpixelComposit(image, white, mask);

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
    auto mask = PixelMap<uint8_t>(9, 3);
    fill(mask);
    mask[1][3] = 255;
    mask[1][4] = 255;
    mask[1][5] = 255;

    auto image = PixelMap<wsRGBA>(3, 3);
    fill(image);

    let color = wsRGBA{ 0.25, 0.50, 0.75, 1.0 };
    subpixelComposit(image, color, mask);

    ASSERT_EQ(image[1][1], color);
}

TEST(PixelMapTests, maskComposit3) {
    auto mask = PixelMap<uint8_t>(9, 3);
    fill(mask);
    mask[1][3] = 0x88;
    mask[1][4] = 0x44;
    mask[1][5] = 0x22;

    auto image = PixelMap<wsRGBA>(3, 3);
    fill(image);

    let white = wsRGBA{ 1.0, 1.0, 1.0, 1.0 };
    subpixelComposit(image, white, mask);

    let alpha = ((0x88 + 0x44 + 0x22) / 3) / 255.0f;
    let red = 0x88 / 255.0f;
    let green = 0x44 / 255.0f;
    let blue = 0x22 / 255.0f;

    let compositColor = wsRGBA{ red / alpha, green / alpha, blue / alpha, alpha };
    ASSERT_EQ(image[1][1].string(), compositColor.string());
}

