// Copyright 2019 Pokitec
// All rights reserved.

#include "PathString.hpp"

namespace TTauri::Draw {

glm::vec2 PathString::advance() const
{
    glm::vec2 totalAdvance = {0.0, 0.0};
    for (size_t i = 0; i < size(); i++) {
        totalAdvance += glyphAdvance(i);
    }
    return totalAdvance;
}

glm::vec2 PathString::ascender() const
{
    glm::vec2 maxAscender = {0.0, 0.0};

    for (size_t i = 0; i < size(); i++) {
        if (glm::length(maxAscender) < glm::length(at(i).ascender)) {
            maxAscender = at(i).ascender;
        }
    }

    return maxAscender;
}

glm::vec2 PathString::descender() const
{
    glm::vec2 maxDescender = {0.0, 0.0};

    for (size_t i = 0; i < size(); i++) {
        if (glm::length(maxDescender) < glm::length(at(i).descender)) {
            maxDescender = at(i).descender;
        }
    }

    return maxDescender;
}

glm::vec2 PathString::capHeight() const
{
    glm::vec2 maxCapHeight = {0.0, 0.0};

    for (size_t i = 0; i < size(); i++) {
        if (glm::length(maxCapHeight) < glm::length(at(i).capHeight)) {
            maxCapHeight = at(i).capHeight;
        }
    }

    return maxCapHeight;
}

glm::vec2 PathString::getStartPosition() const
{
    glm::vec2 v;

    if (alignment == HorizontalAlignment::Left) {
        v = {0.0, 0.0};
    } else if (alignment == HorizontalAlignment::Right) {
        v = -advance();
    } else if (alignment == HorizontalAlignment::Center) {
        v = advance() * -0.5f;
    } else {
        no_default;
    }

    if (alignment == VerticalAlignment::Base) {
        return v;
    } else if (alignment == VerticalAlignment::Bottom) {
        return v - descender();
    } else if (alignment == VerticalAlignment::Top) {
        return v - ascender();
    } else if (alignment == VerticalAlignment::Middle) {
        return v - capHeight() * 0.5f;
    } else {
        no_default;
    }
}

glm::vec2 PathString::cursorAdvance(size_t graphemeIndex) const
{
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

Path PathString::toPath(wsRGBA defaultColor) const
{
    auto r = Path{};

    auto position = getStartPosition();

    // First merge all the non-layered glyphs into a single layer with
    // the default color.
    for (size_t i = 0; i < paths.size(); i++) {
        let path = paths.at(i);
        if (!path.hasLayers()) {
            r += position + path;
        }
        position += glyphAdvance(i);
    }
    r.closeLayer(defaultColor);

    // Next add all the layered glyphs, which have their own colours.
    for (size_t i = 0; i < paths.size(); i++) {
        let path = paths.at(i);
        if (path.hasLayers()) {
            r += position + path;
        }
        position += glyphAdvance(i);
    }

    return r;
}

PathString operator*(glm::mat3x3 const &lhs, PathString rhs)
{
    return rhs *= lhs;
}

PathString &operator*=(PathString &lhs, glm::mat3x3 const &rhs)
{
    for (auto &glyph: lhs.paths) {
        glyph *= rhs;
    }
    return lhs;
}

PathString operator+(Alignment lhs, PathString rhs)
{
    return rhs += lhs;
}

PathString operator+(PathString lhs, Alignment rhs)
{
    return lhs += rhs;
}

PathString &operator+=(PathString &lhs, Alignment rhs)
{
    lhs.alignment = rhs;
    return lhs;
}

}