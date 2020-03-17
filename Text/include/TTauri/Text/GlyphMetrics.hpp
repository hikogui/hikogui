// Copyright 2019, 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/vec.hpp"
#include "TTauri/Foundation/mat.hpp"
#include "TTauri/Foundation/rect.hpp"

namespace TTauri::Text {

/*! Metrics of a glyph.
 * This information is used to position glyphs next to each other
 * and determinate the size of a shaped text.
 */
struct GlyphMetrics {
    /*! Bounding box of the path.
    */
    rect boundingBox = {};

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
        let ligatureRatio = 1.0f / numberOfGraphemes;

        return (advance * ligatureRatio) * static_cast<float>(index);
    }
};

inline GlyphMetrics &operator*=(GlyphMetrics &lhs, mat const &rhs) noexcept
{
    let scale = rhs.scaleX();

    lhs.boundingBox = rhs * lhs.boundingBox;
    lhs.leftSideBearing *= scale; 
    lhs.rightSideBearing *= scale;
    lhs.advance = rhs * lhs.advance;
    lhs.ascender *= scale;
    lhs.descender *= scale;
    lhs.lineGap *= scale;
    lhs.capHeight *= scale;
    lhs.xHeight *= scale;
    return lhs;
}

inline GlyphMetrics &operator*=(GlyphMetrics &lhs, float const rhs) noexcept
{
    lhs.boundingBox *= rhs;
    lhs.leftSideBearing *= rhs;
    lhs.rightSideBearing *= rhs;
    lhs.advance *= rhs;
    lhs.ascender *= rhs;
    lhs.descender *= rhs;
    lhs.lineGap *= rhs;
    lhs.capHeight *= rhs;
    lhs.xHeight *= rhs;
    return lhs;
}

inline GlyphMetrics operator*(mat const &lhs, GlyphMetrics rhs) noexcept
{
    return rhs *= lhs;
}

inline GlyphMetrics operator*(float const lhs, GlyphMetrics rhs) noexcept
{
    return rhs *= lhs;
}

inline GlyphMetrics &operator+=(GlyphMetrics &lhs, vec const &rhs) noexcept
{
    lhs.boundingBox += rhs;
    return lhs;
}

inline GlyphMetrics operator+(vec const &lhs, GlyphMetrics rhs) noexcept
{
    return rhs += lhs;
}

inline GlyphMetrics operator+(GlyphMetrics lhs, vec const &rhs) noexcept
{
    return lhs += rhs;
}


}
