// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "graphic_path.hpp"
#include "bezier_curve.hpp"
#include <hikotest/hikotest.hpp>

TEST_SUITE(graphic_path) {

TEST_CASE(get_beziers_points_of_Layer)
{
    auto path = hi::graphic_path();
    path.moveTo(hi::point2{1, 1});
    path.lineTo(hi::point2{2, 1});
    path.lineTo(hi::point2{2, 2});
    path.lineTo(hi::point2{1, 2});
    path.closeContour();

    auto const beziers = path.getBeziers();
    REQUIRE(beziers.size() == 4);
    REQUIRE(beziers[0] == hi::bezier_curve(hi::point2(1, 1), hi::point2(2, 1)));
    REQUIRE(beziers[1] == hi::bezier_curve(hi::point2(2, 1), hi::point2(2, 2)));
    REQUIRE(beziers[2] == hi::bezier_curve(hi::point2(2, 2), hi::point2(1, 2)));
    REQUIRE(beziers[3] == hi::bezier_curve(hi::point2(1, 2), hi::point2(1, 1)));
}

TEST_CASE(get_bezier_points_of_contour)
{
    auto path = hi::graphic_path();
    path.moveTo(hi::point2{1, 1});
    path.lineTo(hi::point2{2, 1});
    path.lineTo(hi::point2{2, 2});
    path.lineTo(hi::point2{1, 2});
    path.closeContour();

    auto const points = path.getbezier_pointsOfContour(0);
    REQUIRE(points.size() == 4);
    REQUIRE(points[0] == hi::bezier_point(hi::point2(1, 1), hi::bezier_point::Type::Anchor));
    REQUIRE(points[1] == hi::bezier_point(hi::point2(2, 1), hi::bezier_point::Type::Anchor));
    REQUIRE(points[2] == hi::bezier_point(hi::point2(2, 2), hi::bezier_point::Type::Anchor));
    REQUIRE(points[3] == hi::bezier_point(hi::point2(1, 2), hi::bezier_point::Type::Anchor));
}

};
