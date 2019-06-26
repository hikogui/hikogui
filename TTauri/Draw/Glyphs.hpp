// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "attributes.hpp"
#include "Path.hpp"

namespace TTauri::Draw {

struct Glyphs {
    std::vector<Path> glyphs;

    size_t size() const {
        return glyphs.size();
    }

    Path const &at(size_t i) const {
        return glyphs.at(i);
    }

    void add(Path glyph) {
        glyphs.push_back(std::move(glyph));
    }

    glm::vec2 glyphAdvance(size_t i) const {
        return at(i).advance;
    }

    /*! Total width of the text.
     * can be called before positionGlyphs().
     */
    glm::vec2 advance() const {
        glm::vec2 totalAdvance = {0.0, 0.0};
        for (size_t i = 0; i < size(); i++) {
            totalAdvance += glyphAdvance(i);
        }
        return totalAdvance;
    }

    glm::vec2 ascender() const {
        glm::vec2 maxAscender = {0.0, 0.0};

        for (size_t i = 0; i < size(); i++) {
            if (glm::length(maxAscender) < glm::length(at(i).ascender)) {
                maxAscender = at(i).ascender;
            }
        }

        return maxAscender;
    }

    glm::vec2 capHeight() const {
        glm::vec2 maxCapHeight = {0.0, 0.0};

        for (size_t i = 0; i < size(); i++) {
            if (glm::length(maxCapHeight) < glm::length(at(i).capHeight)) {
                maxCapHeight = at(i).capHeight;
            }
        }

        return maxCapHeight;
    }

    /*! Find the start position with a specific alignment.
     */
    glm::vec2 getStartPosition(HorizontalAlignment horizontalAlignment=HorizontalAlignment::Left, VerticalAlignment verticalAlignment=VerticalAlignment::Bottom) const {
        
        glm::vec2 x;

        switch (horizontalAlignment) {
        case HorizontalAlignment::Left:
            x = {0.0, 0.0};
            break;

        case HorizontalAlignment::Right:
            x = -advance();
            break;

        case HorizontalAlignment::Center:
            x = advance() * -0.5f;
            break;

        default:
            no_default;
        }

        switch (verticalAlignment) {
        case VerticalAlignment::Bottom:
            return x;
        case VerticalAlignment::Top:
            return x - capHeight();
        case VerticalAlignment::Middle:
            return x - capHeight() * 0.5f;
        default:
            no_default;
        }
    }

    /*! Get the cursor position at grapheme index.
     */
    glm::vec2 cursorAdvance(size_t graphemeIndex) const {
        auto totalAdvance = glm::vec2{0.0f, 0.0f};

        for (size_t i = 0; i < size(); i++) {
            let &glyph = at(i);
            if (graphemeIndex < glyph.numberOfGraphemes) {
                return totalAdvance + glyph.advanceForGrapheme(graphemeIndex);
            } else {
                totalAdvance += glyphAdvance(i);
            }
            graphemeIndex -= glyph.numberOfGraphemes;
        }
        return totalAdvance;
    }

};

Glyphs operator*(glm::mat3x3 const &lhs, Glyphs rhs);

Glyphs &operator*=(Glyphs &lhs, glm::mat3x3 const &rhs);

}