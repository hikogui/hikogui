// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/AttributedGlyph.hpp"
#include <vector>

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

    size_t size() const noexcept { return line.size(); }

    iterator begin() noexcept { return line.begin(); }
    const_iterator begin() const noexcept { return line.cbegin(); }
    const_iterator cbegin() const noexcept { return line.cbegin(); }

    iterator end() noexcept { return line.end(); }
    const_iterator end() const noexcept { return line.cend(); }
    const_iterator cend() const noexcept { return line.cend(); }

};

}