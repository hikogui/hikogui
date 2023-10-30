// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>

export module hikogui_graphic_path_bezier_point;
import hikogui_geometry;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/*! A point or control-point on contour of bezier curves.
 * The bezier curves can be linear (a line), quadratic or cubic.
 */
export struct bezier_point {
    enum class Type { Anchor, QuadraticControl, CubicControl1, CubicControl2 };

    Type type;
    point2 p;

    constexpr bezier_point(point2 const p, Type const type) noexcept : type(type), p(p) {}

    constexpr bezier_point(float const x, float const y, Type const type) noexcept : bezier_point(point2{x, y}, type) {}

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
    [[nodiscard]] constexpr static std::vector<bezier_point> normalizePoints(
        std::vector<bezier_point>::const_iterator const begin,
        std::vector<bezier_point>::const_iterator const end) noexcept
    {
        std::vector<bezier_point> r;

        hi_axiom((end - begin) >= 2);

        auto prev_it = end - 1;
        auto prev_prev_it = end - 2;
        for (auto it = begin; it != end; it++) {
            switch (it->type) {
            case bezier_point::Type::Anchor:
                hi_axiom(prev_it->type != bezier_point::Type::CubicControl1);
                r.push_back(*it);
                break;

            case bezier_point::Type::QuadraticControl:
                if (it->type == bezier_point::Type::QuadraticControl) {
                    r.emplace_back(midpoint(prev_it->p, it->p), bezier_point::Type::Anchor);

                } else {
                    hi_axiom(prev_it->type == bezier_point::Type::Anchor);
                }
                r.push_back(*it);
                break;

            case bezier_point::Type::CubicControl1:
                r.push_back(*it);
                break;

            case bezier_point::Type::CubicControl2:
                if (prev_it->type == bezier_point::Type::Anchor) {
                    hi_axiom(prev_prev_it->type == bezier_point::Type::CubicControl2);

                    r.emplace_back(reflect(prev_prev_it->p, prev_it->p), bezier_point::Type::CubicControl1);
                } else {
                    hi_axiom(prev_it->type == bezier_point::Type::CubicControl1);
                }
                r.push_back(*it);
                break;

            default:
                hi_no_default();
            }

            prev_prev_it = prev_it;
            prev_it = it;
        }

        for (ssize_t i = 0; i < ssize(r); i++) {
            if (r[i].type == bezier_point::Type::Anchor) {
                std::rotate(r.begin(), r.begin() + i, r.end());
                r.push_back(r.front());
                return r;
            }
        }

        // The result did not contain an anchor.
        hi_no_default();
    }

    [[nodiscard]] constexpr friend bool operator==(bezier_point const&, bezier_point const&) noexcept = default;

    /** Transform the point.
     */
    [[nodiscard]] constexpr friend bezier_point operator*(transformer2 auto const& lhs, bezier_point const& rhs) noexcept
    {
        return {lhs * rhs.p, rhs.type};
    }
};

}} // namespace hi::v1
