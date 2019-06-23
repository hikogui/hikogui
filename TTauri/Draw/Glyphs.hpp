// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "attributes.hpp"
#include "Glyph.hpp"

namespace TTauri::Draw {

struct Glyphs {
    enum class Alignment { Left, Center, Right };

    std::vector<Glyph> glyphs;

    size_t size() const {
        return glyphs.size();
    }

    Glyph const &at(size_t i) const {
        return glyphs.at(i);
    }

    void add(Glyph glyph) {
        glyphs.push_back(std::move(glyph));
    }

    float glyphAdvance(size_t i, float scale=1.0f) const {
        let &glyph = at(i);
        // XXX implement kerning.
        return glyph.advanceWidth * scale;
    }

    glm::vec2 glyphAdvanceVector(size_t i, float scale=1.0f, float rotation=1.0f) const {
        return glm::rotate(glm::vec2{glyphAdvance(i, scale), 0.0f}, rotation);
    }

    /*! Total width of the text.
     * can be called before positionGlyphs().
     */
    float width(float scale=1.0f) const {
        float width = 0.0;
        for (size_t i = 0; i < size(); i++) {
            width += glyphAdvance(i, scale);
        }
        return width;
    }

    glm::vec2 widthVector(float scale=1.0f, float rotation=0.0f) const {
        let v = glm::vec2(width(scale), 0.0f);
        return glm::rotate(v, rotation);
    }

    /*! Find the start position with a specific alignment.
     */
    glm::vec2 getStartPosition(glm::vec2 position, float scale, float rotation=0.0f, HorizontalAlignment alignment=HorizontalAlignment::Left) const {
        switch (alignment) {
        case HorizontalAlignment::Left:
            return position;
        case HorizontalAlignment::Right:
            return position - widthVector(scale, rotation);
        case HorizontalAlignment::Center:
            return position - widthVector(scale, rotation) * 0.5f;
        default:
            abort();
        }
    }

    /*! Get the cursor position at grapheme index.
     */
    float cursorAdvance(size_t graphemeIndex, float scale=1.0f) const {
        float width = 0.0;

        for (size_t i = 0; i < size(); i++) {
            let &glyph = at(i);
            if (graphemeIndex < glyph.numberOfGraphemes) {
                return width + glyph.getAdvanceForGrapheme(graphemeIndex) * scale;
            } else {
                width += glyphAdvance(i, scale);
            }
            graphemeIndex -= glyph.numberOfGraphemes;
        }
        return width;
    }

};

}