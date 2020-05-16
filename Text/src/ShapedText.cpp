// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/small_map.hpp"
#include "TTauri/Text/ShapedText.hpp"
#include "TTauri/Text/globals.hpp"

namespace TTauri::Text {

[[nodiscard]] static std::vector<AttributedGrapheme> makeAttributedGraphemeVector(gstring const &text, TextStyle const &style) noexcept
{
    std::vector<AttributedGrapheme> r;
    r.reserve(ssize(text));

    int index = 0;
    for (let &grapheme: text) {
        r.emplace_back(grapheme, style, index++);
    }

    if (ssize(text) == 0 || text.back() != '\n') {
        r.emplace_back(Grapheme{'\n'}, style, index++);
    }

    return r;
}

[[nodiscard]] static std::vector<AttributedGlyph> graphemes_to_glyphs(std::vector<AttributedGrapheme> const &text) noexcept
{
    // The end-of-paragraph (linefeed) must end text.
    ttauri_assume(ssize(text) >= 1 && text.back().grapheme == '\n');

    std::vector<AttributedGlyph> glyphs;
    glyphs.reserve(size(text));

    AttributedGlyph const *next_glyph = nullptr;

    // Reverse through the text, since the metrics of a glyph depend on the next glyph.
    for (auto i = text.crbegin(); i != text.crend(); ++i) {
        next_glyph = &glyphs.emplace_back(*i, next_glyph);
    }

    // Reverse it back.
    std::reverse(glyphs.begin(), glyphs.end());
    return glyphs;
}

[[nodiscard]] std::pair<float,float> get_cap_and_x_height(std::vector<AttributedGlyph> const &glyphs) noexcept
{
    auto capHeightCounts = small_map<float,int,8>{};
    auto xHeightCounts = small_map<float,int,8>{};

    for (let &g: glyphs) {
        capHeightCounts.increment(g.metrics.capHeight);
        xHeightCounts.increment(g.metrics.xHeight);
    }

    float maxCapHeight = 0.0;
    int maxCapHeightCount = 0;
    for (let &[height, count] : capHeightCounts) {
        if (count > maxCapHeightCount) {
            maxCapHeightCount = count;
            maxCapHeight = height;
        }
    }

    float maxXHeight = 0.0;
    int maxXHeightCount = 0;
    for (let &[height, count] : xHeightCounts) {
        if (count > maxXHeightCount) {
            maxXHeightCount = count;
            maxXHeight = height;
        }
    }

    return {maxCapHeight, maxXHeight};
}

/** Make lines from the glyphs.
 */
[[nodiscard]] static std::vector<AttributedGlyphLine> make_lines(std::vector<AttributedGlyph> &&glyphs) noexcept
{
    std::vector<AttributedGlyphLine> lines;

    auto line_start = glyphs.begin();

    for (auto i = line_start; i != glyphs.end(); ++i) {
        if (i->charClass == GeneralCharacterClass::ParagraphSeparator) {
            // The paragraph seperator stays with the line.
            lines.emplace_back(line_start, i + 1);
            line_start = i + 1;
        }
    }

    glyphs.clear();
    return lines;
}

static void wrap_lines(std::vector<AttributedGlyphLine> &glyphs, float width) noexcept
{
    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        while (i->width > width) {
            // Wrap will modify the current line to the maximum width and return
            // the rest of that line, which we insert here after it.
            i = glyphs.insert(i+1, i->wrap(width)); 
        }
    }
}

/** Calculate the size of the text.
 * @return The extent of the text and the base line position of the middle line.
 */
[[nodiscard]] static vec calculate_text_size(std::vector<AttributedGlyphLine> const &lines) noexcept
{
    auto size = vec{0.0f, 0.0f};

    if (ssize(lines) == 0) {
        return size;
    }

    // Top of first line.
    size = vec{
        lines.front().width,
        lines.front().lineGap + lines.front().ascender
    };

    auto nr_lines = ssize(lines);
    for (ssize_t i = 1; i != nr_lines; ++i) {
        size = vec{
            std::max(size.width(), lines[i].width),
            size.height() + lines[i-1].descender + std::max(lines[i-1].lineGap, lines[i].lineGap) + lines[i].ascender
        };
    }

    // Bottom of last line.
    size.height(size.height() + lines.back().descender + lines.back().lineGap);

    return size;
}

static aarect calculate_bounding_box(std::vector<AttributedGlyphLine> &lines, float width) noexcept
{
    let min_y = lines.back().y - lines.back().descender;
    let max_y = lines.front().y + lines.front().ascender;

    return {
        0.0, min_y,
        width, max_y - min_y
    };
}

static float position_x(Alignment alignment, float line_width, float width) noexcept
{
    if (alignment == HorizontalAlignment::Left) {
        return 0.0f;
    } else if (alignment == HorizontalAlignment::Right) {
        return width - line_width;
    } else if (alignment == HorizontalAlignment::Center) {
        return width * 0.5f - line_width * 0.5f;
    } else {
        no_default;
    }
}

static void position_glyphs(std::vector<AttributedGlyphLine> &lines, Alignment alignment, float width) noexcept
{
    ssize_t start_line_upward;
    ssize_t start_line_downward;
    float start_y_upward = 0.0f;
    float start_y_downward = 0.0f;
    if (alignment == VerticalAlignment::Top || ssize(lines) == 1) {
        start_line_upward = -1; // Don't go upward
        start_line_downward = 0;

    } else if (alignment == VerticalAlignment::Bottom) {
        start_line_upward = ssize(lines) - 1;
        start_line_downward = ssize(lines); // Don't go downward.

    } else if (alignment == VerticalAlignment::Middle) {
        start_line_upward = (ssize(lines) / 2) - 1;
        start_line_downward = ssize(lines) / 2; // The middle line (odd number of lines).

        if (ssize(lines) % 2 == 0) {
            // For even number of lines, the middle is at the gap between the two middle lines.
            let &upward_line = lines[start_line_upward];
            let &downward_line = lines[start_line_downward];
            let gap = std::max(upward_line.lineGap, downward_line.lineGap);

            let baselineDistance = upward_line.descender + gap + downward_line.ascender;
            start_y_upward = 0.5f * baselineDistance;
            start_y_downward = -0.5f * baselineDistance;

        } else {
            // For odd number of lines, the first upward line starts one line higher.
            let &upward_line = lines[start_line_upward];
            let &downward_line = lines[start_line_downward];
            let gap = std::max(upward_line.lineGap, downward_line.lineGap);
            start_y_upward = downward_line.ascender + gap + upward_line.descender;
            start_y_downward = 0.0;
        }
    } else {
        no_default;
    }
    
    {
        // Draw lines downwards.
        float y = start_y_downward;
        bool first_line = true;
        for (ssize_t i = start_line_downward; i != ssize(lines); ++i) {
            auto &line = lines[i];

            if (!first_line) {
                let &prev_line = lines[i-1];
                // Add the descender under the base-line of the previous line.
                y -= prev_line.descender;

                // Add the gap between the two previous and current line.
                y -= std::max(prev_line.lineGap, line.lineGap);

                // Add the ascender above the base-line of the current line.
                y -= line.ascender;
            }
            first_line = false;

            float x = position_x(alignment, line.width, width);
            line.positionGlyphs(vec(x, y));
        }
    }

    {
        // Draw lines upward.
        float y = start_y_upward;
        bool first_line = true;
        for (ssize_t i = start_line_upward; i >= 0; --i) {
            auto &line = lines[i];

            if (!first_line) {
                let &prev_line = lines[i+1];
                // Add the ascender above the base-line of the previous line.
                y += prev_line.ascender;

                // Add the gap between the two previous and current line.
                y += std::max(prev_line.lineGap, line.lineGap);

                // Add the descender below the base-line of the current line.
                y += line.descender;
            }
            first_line = false;

            float x = position_x(alignment, line.width, width);
            line.positionGlyphs(vec(x, y));
        }
    }
}

struct shape_text_result {
    float capHeight;
    float xHeight;
    vec preferedExtent;
    aarect boundingBox;
    std::vector<AttributedGlyphLine> lines;
};

/** Shape the text.
* The given text is in logical-order; the order in which humans write text.
* The resulting glyphs are in left-to-right display order.
*
* The following operations are executed on the text by the `shape_text()` function:
*  - Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
*  - Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
*  - Morph attributed-glyphs using the Font's morph algorithm.
*  - Calculate advance for each attributed-glyph using the Font's advance and kern algorithms.
*  - Add line-breaks to the text to fit within the maximum-width.
*  - Calculate actual size of the text
*  - Align the text within the given extent size.
*
* @param text The text to be shaped.
* @param alignment How the text should be horizontally-aligned inside the maximum_width.
* @param max_width Maximum width that the text should flow into.
* @return prefered size of the text, size of the resulting text, shaped text.
*/
[[nodiscard]] static shape_text_result shape_text(
    std::vector<AttributedGrapheme> text,
    Alignment alignment,
    float width=std::numeric_limits<float>::max()) noexcept
{
    std::vector<AttributedGlyph> attributed_glyphs;

    // Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
    //bidi_algorithm(text);
    ssize_t logicalIndex = 0;
    for (auto &c: text) {
        c.logicalIndex = logicalIndex++;
        c.bidiClass = unicodeData->getBidiClass(c.grapheme[0]);
        c.charClass = to_GeneralCharacterClass(c.bidiClass);
    }
    ttauri_assume(text.back().bidiClass == BidiClass::B);

    // Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
    auto glyphs = graphemes_to_glyphs(text);

    let [capHeight, xHeight] = get_cap_and_x_height(glyphs);

    // Split the text up in lines, based on line-feeds and line-wrapping.
    auto lines = make_lines(std::move(glyphs));

    // Calculate actual size of the box, no smaller than the minimum_size.
    let prefered_extent = calculate_text_size(lines);

    wrap_lines(lines, width);

    // Morph attributed-glyphs using the Font's morph algorithm.
    //morph_glyphs(glyphs);

    // Align the text within the actual box size.
    position_glyphs(lines, alignment, width);

    let bounding_box = calculate_bounding_box(lines, width);

    return {
        capHeight,
        xHeight,
        prefered_extent,
        bounding_box,
        lines
    };
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, Alignment alignment, float width) noexcept :
    alignment(alignment)
{
    auto result = shape_text(text, alignment, width);
    capHeight = result.capHeight;
    xHeight = result.xHeight;
    preferedExtent = result.preferedExtent;
    boundingBox = result.boundingBox;
    lines = std::move(result.lines);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, Alignment alignment, float width) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), alignment, width) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, Alignment alignment, float width) noexcept :
    ShapedText(to_gstring(text), style, alignment, width) {}


[[nodiscard]] ShapedText::const_iterator ShapedText::find(ssize_t index) const noexcept
{
    return std::find_if(cbegin(), cend(), [=](let &x) {
        return x.containsLogicalIndex(index);
    });
}

[[nodiscard]] aarect ShapedText::rectangleOfGrapheme(ssize_t index) const noexcept
{
    let i = find(index);

    // The shaped text will always end with a paragraph separator '\n'.
    // Therefor even if the index points beyond the last character, it will still
    // be on the paragraph separator.
    ttauri_assume(i != cend());

    // We need the line to figure out the ascender/descender height of the line so that
    // the caret does not jump up and down as we walk the text.
    let line_i = i.parent();

    // This is a ligature.
    // The position is inside a ligature.
    // Place the cursor proportional inside the ligature, based on the font-metrics.
    let ligature_index = numeric_cast<int>(i->logicalIndex - index);
    let ligature_advance_left = i->metrics.advanceForGrapheme(ligature_index);
    let ligature_advance_right = i->metrics.advanceForGrapheme(ligature_index + 1);

    let ligature_position_left = i->position + ligature_advance_left;
    let ligature_position_right = i->position + ligature_advance_right;

    let p1 = ligature_position_left - vec(0.0, line_i->descender);
    let p2 = ligature_position_right + vec(0.0, line_i->ascender);
    return aarect::p1p2(p1, p2);
}

[[nodiscard]] aarect ShapedText::leftToRightCaret(ssize_t index, bool insertMode) const noexcept
{
    auto r = rectangleOfGrapheme(index);

    if (insertMode) {
        // Change width to a single pixel.
        r.width(1.0);
    }

    return r;
}

[[nodiscard]] std::vector<aarect> ShapedText::selectionRectangles(ssize_t first, ssize_t last) const noexcept
{
    auto r = std::vector<aarect>{};

    for (ssize_t i = first; i != last; ++i) {
        auto newRect = rectangleOfGrapheme(i);
        if (ssize(r) > 0 && overlaps(r.back(), newRect)) {
            r.back() |= newRect;
        } else {
            r.push_back(newRect);
        }
    }

    return r;
}


[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharAtCoordinate(vec coordinate) const noexcept
{
    for (let &line: lines) {
        auto i = line.find(coordinate);
        if (i == line.cend()) {
            continue;
        }

        if ((i + 1) == line.cend()) {
            // This character is the end of line, or end of paragraph.
            return i->logicalIndex;

        } else {
            let newLogicalIndex = i->relativeIndexAtCoordinate(coordinate);
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

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharOnTheLeft(ssize_t logicalIndex) const noexcept
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

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfCharOnTheRight(ssize_t logicalIndex) const noexcept
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

/** Return the index at the left side of a word
*/
[[nodiscard]] std::pair<ssize_t,ssize_t> ShapedText::indicesOfWord(ssize_t logicalIndex) const noexcept
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
    auto [s, e] = bifind_cluster(cbegin(), cend(), i, [](let &x) {
        return x.isWord() ? 0 : x.isWhiteSpace() ? 1 : 2;
    });

    ttauri_assume(e != i);
    --e;
    return {s->logicalIndex, e->logicalIndex + e->graphemeCount};
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfWordOnTheLeft(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    let [s, e] = indicesOfWord(logicalIndex);
    
    // If the cursor was already on that edge, find the edges of the previous word.
    if (s == logicalIndex) {
        if (let tmp = indexOfCharOnTheLeft(s)) {
            let [s2, e2] = indicesOfWord(*tmp);
            return s2;
        }
    }
    return s;
}

[[nodiscard]] std::optional<ssize_t> ShapedText::indexOfWordOnTheRight(ssize_t logicalIndex) const noexcept
{
    // Find edge of current word.
    let [s, e] = indicesOfWord(logicalIndex);

    // If the cursor was already on that edge, find the edges of the next word.
    if (e == logicalIndex || find(e)->isWhiteSpace()) {
        if (let tmp = indexOfCharOnTheRight(e)) {
            let [s2, e2] = indicesOfWord(*tmp);
            return s2 == e ? e2 : s2;
        }       
    }
    return e;
}

[[nodiscard]] Path ShapedText::get_path() const noexcept
{
    Path r;

    if (ssize(*this) == 0) {
        return r;
    }

    for (let &attr_glyph: *this) {
        r += attr_glyph.get_path();
    }
    r.optimizeLayers();

    return r;
}


}