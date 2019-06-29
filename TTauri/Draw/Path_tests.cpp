// Copyright 2019 Pokitec
// All rights reserved.

#include "Path.hpp"
#include "BezierCurve.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri::Draw;

TEST(PathTests, getBeziersOfLayer) {
    auto path = Path();
    path.moveTo({ 1, 1 });
    path.lineTo({ 2, 1 });
    path.lineTo({ 2, 2 });
    path.lineTo({ 1, 2 });
    path.closeContour();

    let beziers = path.getBeziers();
    ASSERT_EQ(beziers.size(), 4);
    ASSERT_EQ(beziers[0], BezierCurve({ 1,1 }, { 2,1 }));
    ASSERT_EQ(beziers[1], BezierCurve({ 2,1 }, { 2,2 }));
    ASSERT_EQ(beziers[2], BezierCurve({ 2,2 }, { 1,2 }));
    ASSERT_EQ(beziers[3], BezierCurve({ 1,2 }, { 1,1 }));
}

TEST(PathTests, getBezierPointsOfContour) {
    auto path = Path();
    path.moveTo({ 1, 1 });
    path.lineTo({ 2, 1 });
    path.lineTo({ 2, 2 });
    path.lineTo({ 1, 2 });
    path.closeContour();

    let points = path.getBezierPointsOfContour(0);
    ASSERT_EQ(points.size(), 4);
    ASSERT_EQ(points[0], BezierPoint({ 1,1 }, BezierPoint::Type::Anchor));
    ASSERT_EQ(points[1], BezierPoint({ 2,1 }, BezierPoint::Type::Anchor));
    ASSERT_EQ(points[2], BezierPoint({ 2,2 }, BezierPoint::Type::Anchor));
    ASSERT_EQ(points[3], BezierPoint({ 1,2 }, BezierPoint::Type::Anchor));
}