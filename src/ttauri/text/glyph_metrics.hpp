// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../required.hpp"
#include "../numeric_array.hpp"
#include "../aarect.hpp"
#include "../geometry/scale.hpp"

namespace tt {

/*! Metrics of a glyph.
 * This information is used to position glyphs next to each other
 * and determinate the size of a shaped text.
 */
struct glyph_metrics {
    /*! Bounding box of the path.
    */
    aarect boundingBox = {};

    /*! This is the position where the left side of the glyph
    * starts. This includes some leading white space so that the glyph
    * will stand a small distance of the edge.
    *
    * For many glyphs the leftSideBearing is the origin.
    */
    float leftSideBearing = 0.0f;

    /*! This is the position where the right side of the glyph
    * ends. This includes some leading white space so that the glyph
    * will stand a small distance of the edge.
    */
    float rightSideBearing = 0.0f;

    /*! Distance from baseline of highest ascender.
    */
    float ascender = 0.0f;

    /*! Distance from baseline of lowest descender.
    */
    float descender = 0.0f;

    /*! Distance between lines.
     */
    float lineGap = 0.0f;

    /*! Height of capital letter, or height of the letter 'H'.
    */
    float capHeight = 0.0f;

    /*! Height of the small letter 'x'.
    */
    float xHeight = 0.0f;

    /*! The distance to the next character.
    */
    vector2 advance = {0.0f, 0.0f};

    /*! The number of graphemes this glyph represents.
    * This may be larger than one when the glyph is a ligature.
    */
    int numberOfgraphemes = 1;

    glyph_metrics() noexcept = default;
    glyph_metrics(glyph_metrics const &) noexcept = default;
    glyph_metrics(glyph_metrics &&) noexcept = default;
    glyph_metrics &operator=(glyph_metrics const &) noexcept = default;
    glyph_metrics &operator=(glyph_metrics &&) noexcept = default;

    /*! Get the advanceWidth for the specific grapheme of
    * a potential ligature.
    */
    vector2 advanceForgrapheme(int index) const noexcept {
        ttlet ligatureRatio = 1.0f / numberOfgraphemes;

        return advance * ligatureRatio * narrow_cast<float>(index);
    }

    glyph_metrics &scale(float rhs) noexcept
    {
        auto S = scale2(rhs);

        boundingBox = S * boundingBox;
        leftSideBearing *= rhs;
        rightSideBearing *= rhs;
        advance = S * advance;
        ascender *= rhs;
        descender *= rhs;
        lineGap *= rhs;
        capHeight *= rhs;
        xHeight *= rhs;
        return *this;
    }
};



}
