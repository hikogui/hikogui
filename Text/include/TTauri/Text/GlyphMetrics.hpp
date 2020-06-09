// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/aarect.hpp"

namespace TTauri {

/*! Metrics of a glyph.
 * This information is used to position glyphs next to each other
 * and determinate the size of a shaped text.
 */
struct GlyphMetrics {
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
    vec advance = {0.0f, 0.0f};

    /*! The number of graphemes this glyph represents.
    * This may be larger than one when the glyph is a ligature.
    */
    int numberOfGraphemes = 1;

    /*! Get the advanceWidth for the specific grapheme of
    * a potential ligature.
    */
    vec advanceForGrapheme(int index) const noexcept {
        let ligatureRatio = vec{1.0f / numberOfGraphemes};

        return advance * ligatureRatio * vec{index};
    }

    GlyphMetrics &scale(float rhs) noexcept
    {
        auto S = mat::S(rhs, rhs);

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
