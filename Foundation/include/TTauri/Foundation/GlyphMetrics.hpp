// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/geometry.hpp"
#include "TTauri/Foundation/required.hpp"

namespace TTauri {

struct GlyphMetrics {
    /*! Bounding box of the path.
    */
    rect2 boundingBox = {};

    /*! This is the position where the left side of the glyph
    * starts. This includes some leading white space so that the glyph
    * will stand a small distance of the edge.
    *
    * For many glyphs the leftSideBearing is the origin.
    */
    glm::vec2 leftSideBearing = {0.0f, 0.0f};

    /*! This is the position where the right side of the glyph
    * ends. This includes some leading white space so that the glyph
    * will stand a small distance of the edge.
    */
    glm::vec2 rightSideBearing = {0.0f, 0.0f};

    /*! Distance from baseline of highest ascender.
    */
    glm::vec2 ascender = {0.0f, 0.0f};

    /*! Distance from baseline of lowest descender.
    */
    glm::vec2 descender = {0.0f, 0.0f};

    /*! Height of capital letter, or height of the letter 'H'.
    */
    glm::vec2 capHeight = {0.0f, 0.0f};

    /*! Height of the small letter 'x'.
    */
    glm::vec2 xHeight = {0.0f, 0.0f};

    /*! The distance to the next character.
    */
    glm::vec2 advance = {0.0f, 0.0f};

    /*! The number of graphemes this glyph represents.
    * This may be larger than one when the glyph is a ligature.
    */
    int numberOfGraphemes = 1;

    /*! Get the advanceWidth for the specific grapheme of
    * a potential ligature.
    */
    glm::vec2 advanceForGrapheme(int index) const noexcept {
        let ligatureRatio = 1.0f / numberOfGraphemes;

        return (advance * ligatureRatio) * static_cast<float>(index);
    }
};

inline GlyphMetrics &operator*=(GlyphMetrics &lhs, glm::mat3x3 const &rhs) noexcept
{
    lhs.boundingBox *= rhs;
    lhs.leftSideBearing = glm::xy(rhs * glm::vec3(lhs.leftSideBearing, 1.0f));
    lhs.rightSideBearing = glm::xy(rhs * glm::vec3(lhs.rightSideBearing, 1.0f));
    lhs.advance = glm::xy(rhs * glm::vec3(lhs.advance, 0.0f));
    lhs.ascender = glm::xy(rhs * glm::vec3(lhs.ascender, 0.0f));
    lhs.descender = glm::xy(rhs * glm::vec3(lhs.descender, 0.0f));
    lhs.capHeight = glm::xy(rhs * glm::vec3(lhs.capHeight, 0.0f));
    lhs.xHeight = glm::xy(rhs * glm::vec3(lhs.xHeight, 0.0f));
    return lhs;
}

inline GlyphMetrics &operator*=(GlyphMetrics &lhs, float const rhs) noexcept
{
    lhs.boundingBox *= rhs;
    lhs.leftSideBearing = glm::xy(rhs * glm::vec3(lhs.leftSideBearing, 1.0f));
    lhs.rightSideBearing = glm::xy(rhs * glm::vec3(lhs.rightSideBearing, 1.0f));
    lhs.advance = glm::xy(rhs * glm::vec3(lhs.advance, 0.0f));
    lhs.ascender = glm::xy(rhs * glm::vec3(lhs.ascender, 0.0f));
    lhs.descender = glm::xy(rhs * glm::vec3(lhs.descender, 0.0f));
    lhs.capHeight = glm::xy(rhs * glm::vec3(lhs.capHeight, 0.0f));
    lhs.xHeight = glm::xy(rhs * glm::vec3(lhs.xHeight, 0.0f));
    return lhs;
}

inline GlyphMetrics operator*(glm::mat3x3 const &lhs, GlyphMetrics rhs) noexcept
{
    return rhs *= lhs;
}

inline GlyphMetrics operator*(float const lhs, GlyphMetrics rhs) noexcept
{
    return rhs *= lhs;
}

inline GlyphMetrics &operator+=(GlyphMetrics &lhs, glm::vec2 const &rhs) noexcept
{
    lhs.boundingBox += rhs;
    lhs.leftSideBearing += rhs;
    lhs.rightSideBearing += rhs;
    return lhs;
}

inline GlyphMetrics operator+(glm::vec2 const &lhs, GlyphMetrics rhs) noexcept
{
    return rhs += lhs;
}

inline GlyphMetrics operator+(GlyphMetrics lhs, glm::vec2 const &rhs) noexcept
{
    return lhs += rhs;
}


}