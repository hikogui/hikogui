// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/BezierCurve.hpp"
#include <gtest/gtest.h>
#include <iostream>
#include <string>

using namespace std;
using namespace TTauri;

TEST(PathTests, getBeziersOfLayer) {
    auto path = Path();
    path.moveTo(vec::point( 1, 1 ));
    path.lineTo(vec::point( 2, 1 ));
    path.lineTo(vec::point( 2, 2 ));
    path.lineTo(vec::point( 1, 2 ));
    path.closeContour();

    let beziers = path.getBeziers();
    ASSERT_EQ(beziers.size(), 4);
    ASSERT_EQ(beziers[0], BezierCurve(vec::point( 1,1 ), vec::point( 2,1 )));
    ASSERT_EQ(beziers[1], BezierCurve(vec::point( 2,1 ), vec::point( 2,2 )));
    ASSERT_EQ(beziers[2], BezierCurve(vec::point( 2,2 ), vec::point( 1,2 )));
    ASSERT_EQ(beziers[3], BezierCurve(vec::point( 1,2 ), vec::point( 1,1 )));
}

TEST(PathTests, getBezierPointsOfContour) {
    auto path = Path();
    path.moveTo(vec::point( 1, 1 ));
    path.lineTo(vec::point( 2, 1 ));
    path.lineTo(vec::point( 2, 2 ));
    path.lineTo(vec::point( 1, 2 ));
    path.closeContour();

    let points = path.getBezierPointsOfContour(0);
    ASSERT_EQ(points.size(), 4);
    ASSERT_EQ(points[0], BezierPoint(vec::point( 1,1 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[1], BezierPoint(vec::point( 2,1 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[2], BezierPoint(vec::point( 2,2 ), BezierPoint::Type::Anchor));
    ASSERT_EQ(points[3], BezierPoint(vec::point( 1,2 ), BezierPoint::Type::Anchor));
}
