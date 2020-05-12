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
    float width;
    float ascender;
    float descender;
    float lineGap;

    AttributedGlyphLine(vector_type &&line, float width, float ascender, float descender, float lineGap) noexcept :
        line(line), width(width), ascender(ascender), descender(descender), lineGap(lineGap) {}

    [[nodiscard]] AttributedGlyphLine wrap(float maximumWidth) noexcept {
        ttauri_assume(width > maximumWidth);

        auto word_end = line.begin();
        auto word_line_width = 0.0f;

        auto line_width = 0.0f;
        auto i = line.begin();
        for (; i != line.end(); ++i) {
            line_width += i->metrics.advance.x();

            if (line_width > maximumWidth) {
                // Found position where to wrap.
                break;

            } else if (
                i->charClass == GeneralCharacterClass::WhiteSpace ||
                i->charClass == GeneralCharacterClass::ParagraphSeparator
            ) {
                // Include the whitespace in the word, as it should belong at the end of the line.
                word_end = i + 1;
                word_line_width = line_width;
            }
        }

        auto split_position =
            (word_end != line.begin()) ? word_end : // Wrap at word boundary
            (i != line.begin()) ? i : // Wrap at character boundary
            i + 1; // Include at least one character.

        auto reset_of_line = AttributedGlyphLine(split_position, line.end());
        line.erase(split_position, line.cend());
        calculateLineMetrics();
        return reset_of_line;
    }

    [[nodiscard]] aarect boundingBox() const noexcept {
        ttauri_assume(ssize(line) >= 1);

        let p1 = vec::point(
            line.front().position.x(),
            line.front().position.y() - descender
        );

        let p2 = vec::point(
            line.back().position.x() + line.back().metrics.advance.x(),
            line.back().position.y() + ascender
        );

        return aarect::p1p2(p1, p2);
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

private:
    /** This constructor will move the data from first to last.
    */
    AttributedGlyphLine(iterator first, iterator last) noexcept :
        line(), width(0.0f), ascender(0.0f), descender(0.0f), lineGap(0.0f)
    {
        line.reserve(std::distance(first, last));
        std::move(first, last, std::back_inserter(line));
        calculateLineMetrics();
    }

    void calculateLineMetrics() noexcept {
        width = 0.0f;
        ascender = 0.0f;
        descender = 0.0f;
        lineGap = 0.0f;

        for (let &g: line) {
            width += g.metrics.advance.x();
            ascender = std::max(ascender, g.metrics.ascender);
            descender = std::max(descender, g.metrics.descender);
            lineGap = std::max(lineGap, g.metrics.lineGap);
        }
    }

};

}