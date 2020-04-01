// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGlyph.hpp"
#include <vector>
#include <optional>

namespace TTauri::Text {

struct AttributedGlyphLine {
    using vector_type = std::vector<AttributedGlyph>;
    using iterator = vector_type::iterator;
    using const_iterator = vector_type::const_iterator;
    using value_type = vector_type::value_type;

    vector_type line;

    float width = 0.0;
    float ascender = 0.0;
    float descender = 0.0;
    float lineGap = 0.0;
    float xHeight = 0.0;

    AttributedGlyphLine(std::vector<AttributedGlyph>::iterator begin, std::vector<AttributedGlyph>::iterator end, float width) :
        line(begin, end), width(width), ascender(0.0f), descender(0.0f), lineGap(0.0f), xHeight(0.0f)
    {
        for (let &glyph: line) {
            let &metrics = glyph.metrics;

            ascender = std::max(ascender, metrics.ascender);
            descender = std::max(descender, -metrics.descender);
            lineGap = std::max(lineGap, metrics.lineGap);
            xHeight = std::max(xHeight, metrics.xHeight);
        }
    }

    [[nodiscard]] rect boundingBox() const noexcept {
        ttauri_assume(ssize(line) >= 1);

        let p1 = vec::point(
            line.front().position.x(),
            line.front().position.y() - descender
        );

        let p2 = vec::point(
            line.back().position.x() + line.back().metrics.advance.x(),
            line.back().position.y() + ascender
        );

        return rect::p1p2(p1, p2);
    }

    [[nodiscard]] bool contains(vec coordinate) const noexcept {
        return boundingBox().contains(coordinate);            
    }

    [[nodiscard]] const_iterator find(vec coordinate) const noexcept {
        if (!contains(coordinate)) {
            return cend();
        }

        return std::lower_bound(cbegin(), cend(), coordinate.x(), [](let &a, let &b) {
            return (a.position.x() + a.metrics.advance.x()) < b;
        });
    }

    [[nodiscard]] size_t size() const noexcept { return line.size(); }

    [[nodiscard]] iterator begin() noexcept { return line.begin(); }
    [[nodiscard]] const_iterator begin() const noexcept { return line.cbegin(); }
    [[nodiscard]] const_iterator cbegin() const noexcept { return line.cbegin(); }

    [[nodiscard]] iterator end() noexcept { return line.end(); }
    [[nodiscard]] const_iterator end() const noexcept { return line.cend(); }
    [[nodiscard]] const_iterator cend() const noexcept { return line.cend(); }

};

}