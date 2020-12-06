// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/Path.hpp"
#include "ttauri/BezierCurve.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace tt;

TEST(PathTests, getBeziersOfLayer) {
    auto path = Path();
    path.moveTo(f32x4::point( 1, 1 ));
    path.lineTo(f32x4::point( 2, 1 ));
    path.lineTo(f32x4::point( 2, 2 ));
    path.lineTo(f32x4::point( 1, 2 ));
    path.closeContour();

    ttlet beziers = path.getBeziers();
    ASSERT_EQ(beziers.size(), 4);
    ASSERT_EQ(beziers[0], BezierCurve(f32x4::point( 1,1 ), f32x4::point( 2,1 )));
    ASSERT_EQ(beziers[1], BezierCurve(f32x4::point( 2,1 ), f32x4::point( 2,2 )));
    ASSERT_EQ(beziers[2], BezierCurve(f32x4::point( 2,2 ), f32x4::point( 1,2 )));
    ASSERT_EQ(beziers[3], BezierCurve(f32x4::point( 1,2 ), f32x4::point( 1,1 )));
}

TEST(PathTests, getBezierPointsOfContour) {
    auto path = Path();
    path.moveTo(f32x4::point( 1, 1 ));
    path.lineTo(f32x4::point( 2, 1 ));
    path.lineTo(f32x4::point( 2, 2 ));
    path.lineTo(f32x4::point( 1, 2 ));
    path.closeContour();

    ttlet points = path.getBezierPointsOfContour(0);
    ASSERT_EQ(points.size(), 4);
    ASSERT_EQ(points[0], BezierPoint(f32x4::point( 1,1 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[1], BezierPoint(f32x4::point( 2,1 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[2], BezierPoint(f32x4::point( 2,2 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[3], BezierPoint(f32x4::point( 1,2 ), BezierPoint::Type::Anchor));
}
