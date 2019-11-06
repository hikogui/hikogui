// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/PathString.hpp"

namespace TTauri {

glm::vec2 PathString::advance() const noexcept
{
    glm::vec2 totalAdvance = {0.0, 0.0};
    for (int i = 0; i < size(); i++) {
        totalAdvance += glyphAdvance(i);
    }
    return totalAdvance;
}

glm::vec2 PathString::ascender() const noexcept
{
    glm::vec2 maxAscender = {0.0, 0.0};

    for (int i = 0; i < size(); i++) {
        if (glm::length(maxAscender) < glm::length(at(i).metrics.ascender)) {
            maxAscender = at(i).metrics.ascender;
        }
    }

    return maxAscender;
}

glm::vec2 PathString::descender() const noexcept
{
    glm::vec2 maxDescender = {0.0, 0.0};

    for (int i = 0; i < size(); i++) {
        if (glm::length(maxDescender) < glm::length(at(i).metrics.descender)) {
            maxDescender = at(i).metrics.descender;
        }
    }

    return maxDescender;
}

glm::vec2 PathString::capHeight() const noexcept
{
    glm::vec2 maxCapHeight = {0.0, 0.0};

    for (int i = 0; i < size(); i++) {
        if (glm::length(maxCapHeight) < glm::length(at(i).metrics.capHeight)) {
            maxCapHeight = at(i).metrics.capHeight;
        }
    }

    return maxCapHeight;
}

glm::vec2 PathString::getStartPosition() const noexcept
{
    glm::vec2 v;

    if (alignment == HorizontalAlignment::Left) {
        v = glm::vec2{0.0, 0.0};
    } else if (alignment == HorizontalAlignment::Right) {
        v = -advance();
    } else if (alignment == HorizontalAlignment::Center) {
        v = advance() * -0.5f;
    } else {
        no_default;
    }

    if (alignment == VerticalAlignment::Base) {
        // Default.
    } else if (alignment == VerticalAlignment::Bottom) {
        v -= descender();
    } else if (alignment == VerticalAlignment::Top) {
        v -= ascender();
    } else if (alignment == VerticalAlignment::Middle) {
        v -= capHeight() * 0.5f;
    } else {
        no_default;
    }
    return v;
}

glm::vec2 PathString::cursorAdvance(int graphemeIndex) const noexcept
{
    auto totalAdvance = glm::vec2{0.0f, 0.0f};

    for (int i = 0; i < size(); i++) {
        let &glyph = at(i);
        if (graphemeIndex < glyph.metrics.numberOfGraphemes) {
            return totalAdvance + glyph.metrics.advanceForGrapheme(graphemeIndex);
        } else {
            totalAdvance += glyphAdvance(i);
        }
        graphemeIndex -= glyph.metrics.numberOfGraphemes;
    }
    return totalAdvance;
}

Path PathString::toPath(wsRGBA defaultColor) const noexcept
{
    auto r = Path{};

    // First merge all the non-layered glyphs into a single layer with
    // the default color.
    {
        auto position = getStartPosition();
        for (int i = 0; i < paths.size(); i++) {
            let path = paths.at(i);
            if (!path.hasLayers()) {
                r += position + path;
            }
            position += glyphAdvance(i);
        }
        r.closeLayer(defaultColor);
    }

    // Next add all the layered glyphs, which have their own colours.
    {
        auto position = getStartPosition();
        for (int i = 0; i < paths.size(); i++) {
            let path = paths.at(i);
            if (path.hasLayers()) {
                r += position + path;
            }
            position += glyphAdvance(i);
        }
    }

    return r;
}

PathString operator*(glm::mat3x3 const &lhs, PathString rhs) noexcept
{
    return rhs *= lhs;
}

PathString &operator*=(PathString &lhs, glm::mat3x3 const &rhs) noexcept
{
    for (auto &glyph: lhs.paths) {
        glyph *= rhs;
    }
    return lhs;
}

PathString operator+(Alignment lhs, PathString rhs) noexcept
{
    return rhs += lhs;
}

PathString operator+(PathString lhs, Alignment rhs) noexcept
{
    return lhs += rhs;
}

PathString &operator+=(PathString &lhs, Alignment rhs) noexcept
{
    lhs.alignment = rhs;
    return lhs;
}

}
