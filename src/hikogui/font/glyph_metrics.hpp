// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../geometry/module.hpp"

namespace hi::inline v1 {

/*! Metrics of a glyph.
 * This information is used to position glyphs next to each other
 * and determinate the size of a shaped text.
 */
struct glyph_metrics {
    /*! Bounding box of the path.
     */
    aarectangle bounding_rectangle = {};

    /*! This is the position where the left side of the glyph
     * starts. This includes some leading white space so that the glyph
     * will stand a small distance of the edge.
     *
     * For many glyphs the leftSideBearing is the origin.
     */
    float left_side_bearing = 0.0f;

    /*! This is the position where the right side of the glyph
     * ends. This includes some leading white space so that the glyph
     * will stand a small distance of the edge.
     */
    float right_side_bearing = 0.0f;

    /*! The distance to the next character.
     */
    float advance = 0.0f;

    glyph_metrics() noexcept = default;
    glyph_metrics(glyph_metrics const &) noexcept = default;
    glyph_metrics(glyph_metrics &&) noexcept = default;
    glyph_metrics &operator=(glyph_metrics const &) noexcept = default;
    glyph_metrics &operator=(glyph_metrics &&) noexcept = default;

    /** Scale the metrics by a scalar value.
     */
    [[nodiscard]] constexpr friend glyph_metrics operator*(float const &lhs, glyph_metrics const &rhs) noexcept
    {
        glyph_metrics r;
        r.bounding_rectangle = scale2(lhs) * rhs.bounding_rectangle;
        r.left_side_bearing = lhs * rhs.left_side_bearing;
        r.right_side_bearing = lhs * rhs.right_side_bearing;
        r.advance = lhs * rhs.advance;
        return r;
    }
};

} // namespace hi::inline v1
