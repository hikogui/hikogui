// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "attributed_glyph.hpp"
#include <vector>
#include <optional>

namespace tt {

struct attributed_glyph_line {
    using vector_type = std::vector<attributed_glyph>;
    using iterator = vector_type::iterator;
    using const_iterator = vector_type::const_iterator;
    using value_type = vector_type::value_type;

    vector_type line;
    float width;
    float ascender;
    float descender;
    float lineGap;
    float capHeight;
    float xHeight;
    float y;

    /** This constructor will move the data from first to last.
    */
    attributed_glyph_line(iterator first, iterator last) noexcept :
        line(), width(0.0f), ascender(0.0f), descender(0.0f), lineGap(0.0f), capHeight(0.0f), xHeight(0.0f)
    {
        tt_axiom(std::distance(first, last) > 0);

        line.reserve(std::distance(first, last));
        std::move(first, last, std::back_inserter(line));
        calculateLineMetrics();
    }

    [[nodiscard]] bool shouldWrap(float maximum_width) noexcept {
        tt_axiom(std::ssize(line) >= 1);
        return
            width > maximum_width &&
            std::ssize(line) >= (line.back().isParagraphSeparator() ? 3 : 2);
    }

    [[nodiscard]] attributed_glyph_line wrap(float maximum_width) noexcept {
        tt_axiom(shouldWrap(maximum_width));

        auto word_end = line.begin();
        auto line_width = 0.0f;
        auto line_valid_width = 0.0f;

        auto i = line.begin();
        for (; i != line.end(); ++i) {
            line_width += i->metrics.advance.x();
            if (i->isVisible()) {
                line_valid_width = line_width;
            }

            if (line_valid_width > maximum_width) {
                // Found position where to wrap.
                break;

            } else if (i->isWhiteSpace()) {
                // Include the whitespace in the word, as it should belong at the end of the line.
                word_end = i + 1;
            }
        }

        auto split_position =
            (word_end != line.begin()) ? word_end : // Wrap at word boundary
            (i != line.begin()) ? i : // Wrap at character boundary
            i + 1; // Include at least one character.

        auto reset_of_line = attributed_glyph_line(split_position, line.end());
        line.erase(split_position, line.cend());
        calculateLineMetrics();
        return reset_of_line;
    }

    [[nodiscard]] aarectangle boundingBox() const noexcept {
        tt_axiom(std::ssize(line) >= 1);

        ttlet p0 = point2{
            line.front().position.x(),
            line.front().position.y() - descender
        };

        ttlet p3 = point2{
            line.back().position.x() + line.back().metrics.advance.x(),
            line.back().position.y() + ascender
        };

        return aarectangle{p0, p3};
    }

    [[nodiscard]] bool contains(point2 coordinate) const noexcept {
        return boundingBox().contains(coordinate);            
    }

    [[nodiscard]] const_iterator find(point2 coordinate) const noexcept
    {
        auto bbox = boundingBox();

        if (coordinate.y() < bbox.bottom() || coordinate.y() > bbox.top()) {
            return cend();
        }

        if (coordinate.x() < bbox.left()) {
            return cbegin();
        }

        if (coordinate.x() > bbox.right()) {
            return cend() - 1;
        }

        return std::lower_bound(cbegin(), cend(), coordinate.x(), [](ttlet &a, ttlet &b) {
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

    void positionGlyphs(point2 position) noexcept {
        y = position.y();
        for (auto &&g: line) {
            g.position = position;
            position += g.metrics.advance;
        }
    }

private:
    void calculateLineMetrics() noexcept {
        ascender = 0.0f;
        descender = 0.0f;
        lineGap = 0.0f;
        capHeight = 0.0f;
        xHeight = 0.0f;

        auto totalWidth = 0.0f;
        auto validWidth = 0.0f;
        for (ttlet &g: line) {
            totalWidth += g.metrics.advance.x();
            ascender = std::max(ascender, g.metrics.ascender);
            descender = std::max(descender, g.metrics.descender);
            lineGap = std::max(lineGap, g.metrics.lineGap);
            capHeight += g.metrics.capHeight;
            xHeight += g.metrics.xHeight;

            if (g.isVisible()) {
                // Don't include trailing whitespace in the width.
                validWidth = totalWidth;
            }
        }
        capHeight /= narrow_cast<float>(std::ssize(line));
        xHeight /= narrow_cast<float>(std::ssize(line));

        width = validWidth;
    }

};

}
