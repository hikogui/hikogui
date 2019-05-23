// Copyright 2019 Pokitec
// All rights reserved.

#include "PixelMap.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri::Draw;

TEST(PathTests, getQBeziers) {
    auto path = Path();
    path.moveTo({ 1, 1 });
    path.lineTo({ 2, 1 });
    path.lineTo({ 2, 2 });
    path.lineTo({ 1, 2 });
    path.close();

    let beziers = path.getQBeziers();
    ASSERT_EQ(beziers.size(), 4);
    ASSERT_EQ(beziers[0], QBezier({ 1,1 }, { 1.5,1 }, { 2,1 }));
    ASSERT_EQ(beziers[1], QBezier({ 2,1 }, { 2,1.5 }, { 2,2 }));
    ASSERT_EQ(beziers[2], QBezier({ 2,2 }, { 1.5,2 }, { 1,2 }));
    ASSERT_EQ(beziers[3], QBezier({ 1,2 }, { 1,1.5 }, { 1,1 }));
}

TEST(PathTests, getBezierPointsOfSubpath) {
    auto path = Path();
    path.moveTo({ 1, 1 });
    path.lineTo({ 2, 1 });
    path.lineTo({ 2, 2 });
    path.lineTo({ 1, 2 });
    path.close();

    let points = path.getBezierPointsOfSubpath(0);
    ASSERT_EQ(points.size(), 4);
    ASSERT_EQ(points[0], BezierPoint({ 1,1 }, true));
    ASSERT_EQ(points[1], BezierPoint({ 2,1 }, true));
    ASSERT_EQ(points[2], BezierPoint({ 2,2 }, true));
    ASSERT_EQ(points[3], BezierPoint({ 1,2 }, true));
}