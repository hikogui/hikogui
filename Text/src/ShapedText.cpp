// Copyright 2020 Pokitec
// All rights reserved.

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

static void morph_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{

}

/** Make lines from the glyphs.
 */
[[nodiscard]] static std::vector<AttributedGlyphLine> make_lines(std::vector<AttributedGlyph> &&glyphs) noexcept
{
    std::vector<AttributedGlyphLine> lines;

    auto line_start = glyphs.begin();
    auto line_width = 0.0f;
    auto line_ascender = 0.0f;
    auto line_descender = 0.0f;
    auto line_lineGap = 0.0f;

    for (auto i = line_start; i != glyphs.end(); ++i) {
        if (i->charClass == GeneralCharacterClass::ParagraphSeparator) {
            // The paragraph seperator stays with the line.
            auto line_end = i + 1;

            // Move the glyphs into a line.
            auto line = std::vector<AttributedGlyph>{};
            line.reserve(std::distance(line_start, line_end));
            std::move(line_start, line_end, std::back_inserter(line));

            lines.emplace_back(std::move(line), line_width, line_ascender, line_descender, line_lineGap);

            // Reset values for the next line.
            line_start = i + 1;
            line_width = 0.0f;
            line_ascender = 0.0f;
            line_descender = 0.0f;
            line_lineGap = 0.0f;

        } else {
            line_width += i->metrics.advance.x();
            line_ascender = std::max(line_ascender, i->metrics.ascender);
            line_descender = std::max(line_descender, i->metrics.descender);
            line_lineGap = std::max(line_lineGap, i->metrics.lineGap);
        }
    }

    glyphs.clear();
    return lines;
}

static void wrap_lines(std::vector<AttributedGlyphLine> &glyphs, float maximum_width) noexcept
{
    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        while (i->width > maximum_width) {
            // Wrap will modify the current line to the maximum width and return
            // the rest of that line, which we insert here after it.
            i = glyphs.insert(i+1, i->wrap(maximum_width)); 
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
    auto half_nr_lines = nr_lines / 2;
    auto odd_nr_lines = nr_lines % 2 == 1;
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

static void position_glyphs(std::vector<AttributedGlyphLine> &lines, HorizontalAlignment alignment, vec extent) noexcept
{
    // Draw lines from the top-to-down.
    float y = extent.height();
    for (ssize_t i = 0; i != ssize(lines); ++i) {
        auto &line = lines[i];
        if (i == 0) {
            y -= line.lineGap + line.ascender;
        } else {
            let &prev_line = lines[i-1];
            y -= prev_line.descender + std::max(prev_line.lineGap, line.lineGap) + line.ascender;
        }

        float x = 0.0f;
        if (alignment == HorizontalAlignment::Left) {
            x = 0.0f;
        } else if (alignment == HorizontalAlignment::Right) {
            x = extent.width() - line.width;
        } else if (alignment == HorizontalAlignment::Center) {
            x = extent.width() * 0.5f - line.width * 0.5f;
        } else {
            no_default;
        }

        auto position = vec(x, y);
        for (auto &glyph: line) {
            glyph.position = position;
            position += glyph.metrics.advance;
        }
    }
}

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
[[nodiscard]] static std::tuple<vec,vec,std::vector<AttributedGlyphLine>> shape_text(
    std::vector<AttributedGrapheme> text,
    HorizontalAlignment alignment,
    float maximum_width=std::numeric_limits<float>::max()) noexcept
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

    // Split the text up in lines, based on line-feeds and line-wrapping.
    auto lines = make_lines(std::move(glyphs));

    // Calculate actual size of the box, no smaller than the minimum_size.
    let prefered_extent = calculate_text_size(lines);

    wrap_lines(lines, maximum_width);

    // Morph attributed-glyphs using the Font's morph algorithm.
    //morph_glyphs(glyphs);

    // Calculate actual size of the box, no smaller than the minimum_size.
    let extent = calculate_text_size(lines);

    // Align the text within the actual box size.
    position_glyphs(lines, alignment, extent);

    return {prefered_extent, extent, lines};
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, HorizontalAlignment alignment, float maximum_width) noexcept
{
    ttauri_assume((alignment == HorizontalAlignment::Left) || (maximum_width < std::numeric_limits<float>::max()));
    std::tie(this->preferedExtent, this->extent, this->lines) = shape_text(text, alignment, maximum_width);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), alignment, maximum_width) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(to_gstring(text), style, alignment, maximum_width) {}


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