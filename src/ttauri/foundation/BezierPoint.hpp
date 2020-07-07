// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/numeric_cast.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include <vector>

namespace tt {

/*! A point or control-point on contour of bezier curves.
 * The bezier curves can be linear (a line), quadratic or cubic.
 */
struct BezierPoint {
    enum class Type { Anchor, QuadraticControl, CubicControl1, CubicControl2 };

    Type type;
    vec p;

    BezierPoint(vec const p, Type const type) noexcept : type(type), p(p) {
        tt_assume(p.is_point());
    }

    BezierPoint(float const x, float const y, Type const type) noexcept : BezierPoint(vec::point(x, y), type) {}

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
    [[nodiscard]] static std::vector<BezierPoint> normalizePoints(
        std::vector<BezierPoint>::const_iterator const begin,
        std::vector<BezierPoint>::const_iterator const end
    ) noexcept {
        std::vector<BezierPoint> r;

        tt_assert((end - begin) >= 2);

        auto previousPoint = *(end - 1);
        auto previousPreviousPoint = *(end - 2);
        for (auto i = begin; i != end; i++) {
            ttlet point = *i;

            switch (point.type) {
            case BezierPoint::Type::Anchor:
                tt_assert(previousPoint.type != BezierPoint::Type::CubicControl1);
                r.push_back(point);
                break;

            case BezierPoint::Type::QuadraticControl:
                if (previousPoint.type == BezierPoint::Type::QuadraticControl) {
                    r.emplace_back(midpoint(previousPoint.p, point.p), BezierPoint::Type::Anchor);

                } else {
                    tt_assert(previousPoint.type == BezierPoint::Type::Anchor);
                }
                r.push_back(point);
                break;

            case BezierPoint::Type::CubicControl1:
                r.push_back(point);
                break;

            case BezierPoint::Type::CubicControl2:
                if (previousPoint.type == BezierPoint::Type::Anchor) {
                    tt_assert(previousPreviousPoint.type == BezierPoint::Type::CubicControl2);

                    r.emplace_back(reflect_point(previousPreviousPoint.p, previousPoint.p), BezierPoint::Type::CubicControl1);
                } else {
                    tt_assert(previousPoint.type == BezierPoint::Type::CubicControl1);
                }
                r.push_back(point);
                break;

            default:
                tt_no_default;
            }

            previousPreviousPoint = previousPoint;
            previousPoint = point;
        }

        for (ssize_t i = 0; i < ssize(r); i++) {
            if (r[i].type == BezierPoint::Type::Anchor) {
                std::rotate(r.begin(), r.begin() + i, r.end());
                r.push_back(r.front());
                return r;
            }
        }

        // The result did not contain an anchor.
        tt_assert(false);
    }

    /** Transform the point.
     */
    template<typename M, std::enable_if_t<is_mat_v<M>, int> = 0>
    inline BezierPoint &operator*=(M const &rhs) noexcept {
        p = rhs * p;
        return *this;
    }

    [[nodiscard]] friend bool operator==(BezierPoint const &lhs, BezierPoint const &rhs) noexcept {
        return (lhs.p == rhs.p) && (lhs.type == rhs.type);
    }

    /** Transform the point.
    */
    template<typename M, std::enable_if_t<is_mat_v<M>, int> = 0>
    [[nodiscard]] friend BezierPoint operator*(M const &lhs, BezierPoint const &rhs) noexcept {
        return { lhs * rhs.p, rhs.type };
    }
};

}
