// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "cast.hpp"
#include "numeric_array.hpp"
#include "geometry/transform.hpp"
#include <vector>

namespace tt {

/*! A point or control-point on contour of bezier curves.
 * The bezier curves can be linear (a line), quadratic or cubic.
 */
struct bezier_point {
    enum class Type { Anchor, QuadraticControl, CubicControl1, CubicControl2 };

    Type type;
    point2 p;

    bezier_point(point2 const p, Type const type) noexcept : type(type), p(p)
    {
    }

    bezier_point(float const x, float const y, Type const type) noexcept : bezier_point(point2{x, y}, type) {}

    /*! Normalize points in a list.
     * The following normalizations are executed:
     *  - Missing anchor points between two quadratic-control-points are added.
     *  - Missing first-cubic-control-points are added by reflecting the previous
     *    second-control point around the previous anchor.
     *  - The list of points will start with an anchor.
     *  - The list will close with the first anchor.
     *
     * \param begin iterator to the start of the bezier point list.
     * \param end iterator beyond the end of the bezier point list.
     * \return a vector of bezier point that include all the anchor and control points.
     */
    [[nodiscard]] static std::vector<bezier_point> normalizePoints(
        std::vector<bezier_point>::const_iterator const begin,
        std::vector<bezier_point>::const_iterator const end) noexcept
    {
        std::vector<bezier_point> r;

        tt_assert((end - begin) >= 2);

        auto previousPoint = *(end - 1);
        auto previousPreviousPoint = *(end - 2);
        for (auto i = begin; i != end; i++) {
            ttlet point = *i;

            switch (point.type) {
            case bezier_point::Type::Anchor:
                tt_assert(previousPoint.type != bezier_point::Type::CubicControl1);
                r.push_back(point);
                break;

            case bezier_point::Type::QuadraticControl:
                if (previousPoint.type == bezier_point::Type::QuadraticControl) {
                    r.emplace_back(midpoint(previousPoint.p, point.p), bezier_point::Type::Anchor);

                } else {
                    tt_assert(previousPoint.type == bezier_point::Type::Anchor);
                }
                r.push_back(point);
                break;

            case bezier_point::Type::CubicControl1: r.push_back(point); break;

            case bezier_point::Type::CubicControl2:
                if (previousPoint.type == bezier_point::Type::Anchor) {
                    tt_assert(previousPreviousPoint.type == bezier_point::Type::CubicControl2);

                    r.emplace_back(reflect(previousPreviousPoint.p, previousPoint.p), bezier_point::Type::CubicControl1);
                } else {
                    tt_assert(previousPoint.type == bezier_point::Type::CubicControl1);
                }
                r.push_back(point);
                break;

            default: tt_no_default();
            }

            previousPreviousPoint = previousPoint;
            previousPoint = point;
        }

        for (ssize_t i = 0; i < std::ssize(r); i++) {
            if (r[i].type == bezier_point::Type::Anchor) {
                std::rotate(r.begin(), r.begin() + i, r.end());
                r.push_back(r.front());
                return r;
            }
        }

        // The result did not contain an anchor.
        tt_assert(false);
    }

    [[nodiscard]] friend bool operator==(bezier_point const &lhs, bezier_point const &rhs) noexcept
    {
        return (lhs.p == rhs.p) && (lhs.type == rhs.type);
    }

    /** Transform the point.
     */
    [[nodiscard]] friend bezier_point operator*(geo::transformer auto const &lhs, bezier_point const &rhs) noexcept {
        return {lhs * rhs.p, rhs.type};
    }
};

} // namespace tt
