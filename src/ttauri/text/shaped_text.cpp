// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "shaped_text.hpp"
#include "unicode_description.hpp"
#include "../small_map.hpp"
#include "../application.hpp"

namespace tt {

[[nodiscard]] static std::vector<attributed_grapheme> makeattributed_graphemeVector(gstring const &text, text_style const &style) noexcept
{
    std::vector<attributed_grapheme> r;
    r.reserve(std::ssize(text));

    int index = 0;
    for (ttlet &grapheme: text) {
        r.emplace_back(grapheme, style, index++);
    }

    if (std::ssize(text) == 0 || text.back() != grapheme::PS()) {
        r.emplace_back(grapheme::PS(), style, index++);
    }

    return r;
}

[[nodiscard]] static std::vector<attributed_glyph> graphemes_to_glyphs(std::vector<attributed_grapheme> const &text) noexcept
{
    // The end-of-paragraph must end text.
    tt_axiom(std::ssize(text) >= 1 && text.back().grapheme == grapheme::PS());

    std::vector<attributed_glyph> glyphs;
    glyphs.reserve(size(text));

    attributed_glyph const *next_glyph = nullptr;

    // Reverse through the text, since the metrics of a glyph depend on the next glyph.
    for (auto i = text.crbegin(); i != text.crend(); ++i) {
        next_glyph = &glyphs.emplace_back(*i, next_glyph);
    }

    // Reverse it back.
    std::reverse(glyphs.begin(), glyphs.end());
    return glyphs;
}

/** Make lines from the glyphs.
 */
[[nodiscard]] static std::vector<attributed_glyph_line> make_lines(std::vector<attributed_glyph> &&glyphs) noexcept
{
    std::vector<attributed_glyph_line> lines;

    auto line_start = glyphs.begin();

    for (auto i = line_start; i != glyphs.end(); ++i) {
        if (i->general_category == unicode_general_category::Zp) {
            // The paragraph separator stays with the line.
            lines.emplace_back(line_start, i + 1);
            line_start = i + 1;
        }
    }

    glyphs.clear();
    return lines;
}

static void wrap_lines(std::vector<attributed_glyph_line> &lines, float width) noexcept
{
    for (auto i = lines.begin(); i != lines.end(); ++i) {
        while (i->shouldWrap(width)) {
            // Wrap will modify the current line to the maximum width and return
            // the rest of that line, which we insert here after it.
            i = lines.insert(i+1, i->wrap(width)); 
        }
    }
}

/** Calculate the size of the text.
 * @return The extent of the text and the base line position of the middle line.
 */
[[nodiscard]] static extent2 calculate_text_size(std::vector<attributed_glyph_line> const &lines) noexcept
{
    auto size = extent2{0.0f, 0.0f};

    if (std::ssize(lines) == 0) {
        return size;
    }

    // Top of first line.
    size = extent2{
        lines.front().width,
        lines.front().lineGap + lines.front().ascender
    };

    auto nr_lines = std::ssize(lines);
    for (ssize_t i = 1; i != nr_lines; ++i) {
        size = extent2{
            std::max(size.width(), lines[i].width),
            size.height() + lines[i-1].descender + std::max(lines[i-1].lineGap, lines[i].lineGap) + lines[i].ascender
        };
    }

    // Bottom of last line.
    size.height() = size.height() + lines.back().descender + lines.back().lineGap;

    return size;
}

[[nodiscard]] static aarectangle calculate_bounding_box(std::vector<attributed_glyph_line> const &lines, float width) noexcept
{
    ttlet min_y = lines.back().y - lines.back().descender;
    ttlet max_y = lines.front().y + lines.front().ascender;

    return {
        0.0, min_y,
        width, max_y - min_y
    };
}

[[nodiscard]] static float position_x(alignment alignment, float line_width, float width) noexcept
{
    if (alignment == horizontal_alignment::left) {
        return 0.0f;
    } else if (alignment == horizontal_alignment::right) {
        return width - line_width;
    } else if (alignment == horizontal_alignment::center) {
        return width * 0.5f - line_width * 0.5f;
    } else {
        tt_no_default();
    }
}

static void position_glyphs(std::vector<attributed_glyph_line> &lines, alignment alignment, float width) noexcept
{
    ssize_t start_line_upward;
    ssize_t start_line_downward;
    float start_y_upward = 0.0f;
    float start_y_downward = 0.0f;
    if (alignment == vertical_alignment::top || std::ssize(lines) == 1) {
        start_line_upward = -1; // Don't go upward
        start_line_downward = 0;

    } else if (alignment == vertical_alignment::bottom) {
        start_line_upward = std::ssize(lines) - 1;
        start_line_downward = std::ssize(lines); // Don't go downward.

    } else if (alignment == vertical_alignment::middle) {
        start_line_upward = (std::ssize(lines) / 2) - 1;
        start_line_downward = std::ssize(lines) / 2; // The middle line (odd number of lines).

        if (std::ssize(lines) % 2 == 0) {
            // For even number of lines, the middle is at the gap between the two middle lines.
            ttlet &upward_line = lines[start_line_upward];
            ttlet &downward_line = lines[start_line_downward];
            ttlet gap = std::max(upward_line.lineGap, downward_line.lineGap);

            ttlet baselineDistance = upward_line.descender + gap + downward_line.ascender;
            start_y_upward = 0.5f * baselineDistance;
            start_y_downward = -0.5f * baselineDistance;

        } else {
            // For odd number of lines, the first upward line starts one line higher.
            ttlet &upward_line = lines[start_line_upward];
            ttlet &downward_line = lines[start_line_downward];
            ttlet gap = std::max(upward_line.lineGap, downward_line.lineGap);
            start_y_upward = downward_line.ascender + gap + upward_line.descender;
            start_y_downward = 0.0;
        }
    } else {
        tt_no_default();
    }
    
    {
        // Draw lines downwards.
        float y = start_y_downward;
        bool first_line = true;
        for (ssize_t i = start_line_downward; i != std::ssize(lines); ++i) {
            auto &line = lines[i];

            if (!first_line) {
                ttlet &prev_line = lines[i-1];
                // Add the descender under the base-line of the previous line.
                y -= prev_line.descender;

                // Add the gap between the two previous and current line.
                y -= std::max(prev_line.lineGap, line.lineGap);

                // Add the ascender above the base-line of the current line.
                y -= line.ascender;
            }
            first_line = false;

            float x = position_x(alignment, line.width, width);
            line.positionGlyphs(point2{x, y});
        }
    }

    {
        // Draw lines upward.
        float y = start_y_upward;
        bool first_line = true;
        for (ssize_t i = start_line_upward; i >= 0; --i) {
            auto &line = lines[i];

            if (!first_line) {
                ttlet &prev_line = lines[i+1];
                // Add the ascender above the base-line of the previous line.
                y += prev_line.ascender;

                // Add the gap between the two previous and current line.
                y += std::max(prev_line.lineGap, line.lineGap);

                // Add the descender below the base-line of the current line.
                y += line.descender;
            }
            first_line = false;

            float x = position_x(alignment, line.width, width);
            line.positionGlyphs(point2{x, y});
        }
    }
}

struct shape_text_result {
    extent2 preferred_extent;
    aarectangle boundingBox;
    std::vector<attributed_glyph_line> lines;
};

/** Shape the text.
* The given text is in logical-order; the order in which humans write text.
* The resulting glyphs are in left-to-right display order.
*
* The following operations are executed on the text by the `shape_text()` function:
*  - Put graphemes in left-to-right display order using the unicode_data::global's bidi_algorithm.
*  - Convert attributed-graphemes into attributes-glyphs using font_book's find_glyph algorithm.
*  - Morph attributed-glyphs using the font's morph algorithm.
*  - Calculate advance for each attributed-glyph using the font's advance and kern algorithms.
*  - Add line-breaks to the text to fit within the maximum-width.
*  - Calculate actual size of the text
*  - Align the text within the given extent size.
*
* @param text The text to be shaped.
* @param alignment How the text should be horizontally-aligned inside the maximum_width.
* @param max_width Maximum width that the text should flow into.
* @return preferred size of the text, size of the resulting text, shaped text.
*/
[[nodiscard]] static shape_text_result shape_text(
    std::vector<attributed_grapheme> text,
    float width,
    alignment alignment,
    float wrap) noexcept
{
    std::vector<attributed_glyph> attributed_glyphs;

    // Put graphemes in left-to-right display order using the unicode_data::global's bidi_algorithm.
    //bidi_algorithm(text);
    ssize_t logicalIndex = 0;
    for (auto &c: text) {
        ttlet &description = unicode_description_find(c.grapheme[0]);
        c.logicalIndex = logicalIndex++;
        c.bidi_class = description.bidi_class();
        c.general_category = description.general_category();
    }
    tt_axiom(text.back().general_category == unicode_general_category::Zp);

    // Convert attributed-graphemes into attributes-glyphs using font_book's find_glyph algorithm.
    auto glyphs = graphemes_to_glyphs(text);

    // Split the text up in lines, based on line-feeds and line-wrapping.
    auto lines = make_lines(std::move(glyphs));

    // Calculate actual size of the box, no smaller than the minimum_size.
    ttlet preferred_extent = ceil(calculate_text_size(lines));

    if (wrap) {
        wrap_lines(lines, width);
    }

    // Morph attributed-glyphs using the font's morph algorithm.
    //morph_glyphs(glyphs);

    // Align the text within the actual box size.
    position_glyphs(lines, alignment, width);

    ttlet bounding_box = calculate_bounding_box(lines, width);

    return {
        preferred_extent,
        bounding_box,
        lines
    };
}


shaped_text::shaped_text(
    std::vector<attributed_grapheme> const &text,
    float width,
    tt::alignment alignment,
    bool wrap
) noexcept :
    alignment(alignment),
    width(width)
{
    auto result = shape_text(text, width, alignment, wrap);
    preferred_extent = result.preferred_extent;
    boundingBox = result.boundingBox;
    lines = std::move(result.lines);
}

shaped_text::shaped_text(
    gstring const &text,
    text_style const &style,
    float width,
    tt::alignment alignment,
    bool wrap)
noexcept :
    shaped_text(makeattributed_graphemeVector(text, style), width, alignment, wrap) {}

shaped_text::shaped_text(
    std::u8string_view text,
    text_style const &style,
    float width,
    tt::alignment alignment,
    bool wrap
) noexcept :
    shaped_text(to_gstring(text), style, width, alignment, wrap) {}


[[nodiscard]] shaped_text::const_iterator shaped_text::find(ssize_t index) const noexcept
{
    return std::find_if(cbegin(), cend(), [=](ttlet &x) {
        return x.containsLogicalIndex(index);
    });
}

[[nodiscard]] aarectangle shaped_text::rectangleOfgrapheme(ssize_t index) const noexcept
{
    ttlet i = find(index);

    // The shaped text will always end with a paragraph separator '\n'.
    // Therefor even if the index points beyond the last character, it will still
    // be on the paragraph separator.
    tt_axiom(i != cend());

    // We need the line to figure out the ascender/descender height of the line so that
    // the caret does not jump up and down as we walk the text.
    ttlet line_i = i.parent();

    // This is a ligature.
    // The position is inside a ligature.
    // Place the cursor proportional inside the ligature, based on the font-metrics.
    ttlet ligature_index = narrow_cast<int>(i->logicalIndex - index);
    ttlet ligature_advance_left = i->metrics.advanceForgrapheme(ligature_index);
    ttlet ligature_advance_right = i->metrics.advanceForgrapheme(ligature_index + 1);

    ttlet ligature_position_left = i->position + ligature_advance_left;
    ttlet ligature_position_right = i->position + ligature_advance_right;

    ttlet p0 = ligature_position_left - vector2{0.0f, line_i->descender};
    ttlet p3 = ligature_position_right + vector2{0.0f, line_i->ascender};
    return aarectangle{p0, p3};
}

[[nodiscard]] aarectangle shaped_text::leftToRightCaret(ssize_t index, bool insertMode) const noexcept
{
    auto r = rectangleOfgrapheme(index);

    if (insertMode) {
        // Change width to a single pixel.
        r.set_width(1.0);
    }

    return r;
}

[[nodiscard]] std::vector<aarectangle> shaped_text::selectionRectangles(ssize_t first, ssize_t last) const noexcept
{
    auto r = std::vector<aarectangle>{};

    for (ssize_t i = first; i != last; ++i) {
        auto newRect = rectangleOfgrapheme(i);
        if (std::ssize(r) > 0 && overlaps(r.back(), newRect)) {
            r.back() |= newRect;
        } else {
            r.push_back(newRect);
        }
    }

    return r;
}


[[nodiscard]] std::optional<ssize_t> shaped_text::indexOfCharAtCoordinate(point2 coordinate) const noexcept
{
    for (ttlet &line: lines) {
        auto i = line.find(coordinate);
        if (i == line.cend()) {
            continue;
        }

        if ((i + 1) == line.cend()) {
            // This character is the end of line, or end of paragraph.
            return i->logicalIndex;

        } else {
            ttlet newLogicalIndex = i->relativeIndexAtCoordinate(coordinate);
            if (newLogicalIndex < 0) {
                return i->logicalIndex;
            } else if (newLogicalIndex >= i->graphemeCount) {
                // Closer to the next glyph.
                return (i+1)->logicalIndex;
            } else {
                return i->logicalIndex + newLogicalIndex;
            }
        }
    }
    return {};
}

[[nodiscard]] std::optional<ssize_t> shaped_text::indexOfCharOnTheLeft(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);
    if (i == cbegin()) {
        return {};
    } else if (logicalIndex != i->logicalIndex) {
        // Go left inside a ligature.
        return logicalIndex - 1;
    } else {
        --i;
        return i->logicalIndex + i->graphemeCount - 1;
    }
}

[[nodiscard]] std::optional<ssize_t> shaped_text::indexOfCharOnTheRight(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);
    if (i->isParagraphSeparator()) {
        return {};
    } else if (logicalIndex < (i->logicalIndex + i->graphemeCount)) {
        // Go right inside a ligature.
        return logicalIndex + 1;
    } else {
        ++i;
        return i->logicalIndex;
    }
}

[[nodiscard]] std::pair<ssize_t,ssize_t> shaped_text::indicesOfParagraph(ssize_t logicalIndex) const noexcept
{
    tt_axiom(size() != 0);
    if (size() == 1) {
        // One line with only a paragraph separator means this is empty.
        return {0,0};
    }

    auto i = find(logicalIndex);

    auto beginOfParagraph = i;
    while (true) {
        if (beginOfParagraph == cbegin()) {
            break;
        }

        if ((beginOfParagraph - 1)->isParagraphSeparator()) {
            break;
        }
        --beginOfParagraph;
    }

    auto endOfParagraph = i;
    while (true) {
        if (endOfParagraph->isParagraphSeparator()) {
            break;
        }
        ++endOfParagraph;
        tt_axiom(endOfParagraph != cend());
    }

    tt_axiom(beginOfParagraph != endOfParagraph);
    auto lastCharacter = endOfParagraph - 1;
    return {beginOfParagraph->logicalIndex, lastCharacter->logicalIndex + lastCharacter->graphemeCount};
}

/** Return the index at the left side of a word
*/
[[nodiscard]] std::pair<ssize_t,ssize_t> shaped_text::indicesOfWord(ssize_t logicalIndex) const noexcept
{
    auto i = find(logicalIndex);

    // If the position is the paragraph separator, adjust to one glyph to the left.
    if (i->isParagraphSeparator()) {
        if (i == cbegin()) {
            return {0, 0};
        } else {
            --i;
        }
    }

    if (i->isWhiteSpace()) {
        if (i == cbegin()) {
            // Whitespace at start of line is counted as a word.
            ;
        } else if (!(i-1)->isWhiteSpace()) {
            // The glyph on the left is not a white space, means we need to select the word on the left
            --i;
        } else {
            // Double white space select all the white spaces in a row.
            ;
        }
    }

    // Expand the word to left and right.
    auto [s, e] = bifind_cluster(cbegin(), cend(), i, [](ttlet &x) {
        return x.selectionWordClusterID();
    });

    tt_axiom(e != i);
    --e;
    return {s->logicalIndex, e->logicalIndex + e->graphemeCount};
}

[[nodiscard]] std::optional<ssize_t> shaped_text::indexOfWordOnTheLeft(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    ttlet [s, e] = indicesOfWord(logicalIndex);
    
    // If the cursor was already on that edge, find the edges of the previous word.
    if (s == logicalIndex) {
        if (ttlet tmp = indexOfCharOnTheLeft(s)) {
            ttlet [s2, e2] = indicesOfWord(*tmp);
            return s2;
        }
    }
    return s;
}

[[nodiscard]] std::optional<ssize_t> shaped_text::indexOfWordOnTheRight(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    ttlet [s, e] = indicesOfWord(logicalIndex);

    // If the cursor was already on that edge, find the edges of the next word.
    if (e == logicalIndex || find(e)->isWhiteSpace()) {
        if (ttlet tmp = indexOfCharOnTheRight(e)) {
            ttlet [s2, e2] = indicesOfWord(*tmp);
            return s2 == e ? e2 : s2;
        }       
    }
    return e;
}

[[nodiscard]] graphic_path shaped_text::get_path() const noexcept
{
    graphic_path r;

    if (std::ssize(*this) == 0) {
        return r;
    }

    for (ttlet &attr_glyph: *this) {
        r += attr_glyph.get_path();
    }
    r.optimizeLayers();

    return r;
}


}