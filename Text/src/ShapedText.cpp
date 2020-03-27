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

static void bidi_algorithm(std::vector<AttributedGrapheme> &text) noexcept
{
    // First remember the logical position before reordering the glyphs.
    ssize_t logicalIndex = 0;
    for (auto &ag: text) {
        ag.logicalIndex = logicalIndex++;
    }
}

[[nodiscard]] static std::vector<AttributedGlyph> graphemes_to_glyphs(std::vector<AttributedGrapheme> const &text) noexcept
{
    // The end-of-paragraph (linefeed) must end text.
    ttauri_assume(ssize(text) >= 1 && text.back().grapheme == '\n');

    std::vector<AttributedGlyph> glyphs;
    glyphs.reserve(size(text));

    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    for (let &ag: text) {
        let font_id = font_book.find_font(ag.style.family_id, ag.style.variant);

        // The end-of-paragraph is represented by a space glyph, which is usefull for
        // producing a correct cursor at an empty line at the end of a paragraph.
        let g = (ag.grapheme == '\n') ? Grapheme{0} : ag.grapheme;
        glyphs.emplace_back(ag, font_book.find_glyph(font_id, g));
    }

    return glyphs;
}

static void morph_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{

}

static void load_metrics_for_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{
    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    auto font_id = FontID{};
    Font const *font = nullptr;
    auto next_i = glyphs.begin();
    for (auto i = next_i; i != glyphs.end(); i = next_i) {
        next_i = i + 1;

        // Get a pointer to the actual font, when the font_id of the glyph changes.
        let new_font_id = i->glyphs.font_id();
        if (font_id != new_font_id) {
            font_id = new_font_id;
            font = &(font_book.get_font(font_id));
        }
        ttauri_assume(font != nullptr);
        let next_is_same_font = (next_i != glyphs.end()) && (next_i->glyphs.font_id() == font_id);

        // Get the metrics of the main glyph.
        let first_glyph = i->glyphs.front();
        let next_glyph = next_is_same_font ? next_i->glyphs.front() : GlyphID{};

        if (!font->loadGlyphMetrics(first_glyph, i->metrics, next_glyph)) {
            LOG_ERROR("Could not load metrics for glyph {} in font {} - {}", static_cast<int>(i->glyphs[0]), font->description.family_name, font->description.sub_family_name);
            // failed to load metrics. Switch to glyph zero and load again.
            i->glyphs.clear();
            i->glyphs.set_font_id(font_id);
            i->glyphs += GlyphID{0};
            if (!font->loadGlyphMetrics(i->glyphs[0], i->metrics)) {
                // Using null-metrics when even the null-glyph can not be found.
                LOG_ERROR("Could not load metrics for null-glyph in font {} - {}", font->description.family_name, font->description.sub_family_name);
            }
        }

        // Scale the metrics according to font-size of this glyph.
        i->metrics *= i->style.size;

        // XXX merge the bounding box of combining glyphs in the metrics.
    }
}

[[nodiscard]] static std::vector<AttributedGlyphLine> make_lines(std::vector<AttributedGlyph> &glyphs, float maximum_width) noexcept
{
    std::vector<AttributedGlyphLine> lines;

    float width = 0.0f;
    auto start_of_line = glyphs.begin();
    auto end_of_word = glyphs.begin();
    float end_of_word_width = 0.0f;

    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        if (i->breakableWhitespace) {
            // When a line is created the whitespace should be appended to the
            // end-of-line so cursor position can still be calculated correctly.
            end_of_word = i + 1;
            // For the width we do not count the whitespace after the word.
            end_of_word_width = width;
        }

        if (!i->endOfParagraph) {
            // Do not include the width of the endOfParagraph marker.
            width += i->metrics.advance.x();
        }

        if ((width > maximum_width) && (start_of_line != end_of_word)) {
            // Line is to long, and exists of at least a full word.
            lines.emplace_back(start_of_line, end_of_word, end_of_word_width);

            // Skip back in the for-loop.
            i = end_of_word;
            start_of_line = end_of_word;
            end_of_word = end_of_word;
            end_of_word_width = 0.0f;
        }
    }

    if (start_of_line != glyphs.end()) {
        // Any whitespace at the end of the paragraph should be kept in the last line.
        lines.emplace_back(start_of_line, glyphs.end(), width);
    }

    return lines;
}

/** Calculate the size of the text.
 * @return The extent of the text and the base line position of the middle line.
 */
[[nodiscard]] static std::pair<vec,float> calculate_text_size(std::vector<AttributedGlyphLine> const &lines) noexcept
{
    auto size = vec{0.0f, 0.0f};

    if (ssize(lines) == 0) {
        return {size, 0.0f};
    }

    // Top of first line.
    size = vec{
        lines.front().width,
        lines.front().lineGap + lines.front().ascender
    };
    auto base_line = size.height();

    auto nr_lines = ssize(lines);
    auto half_nr_lines = nr_lines / 2;
    auto odd_nr_lines = nr_lines % 2 == 1;
    for (ssize_t i = 1; i != nr_lines; ++i) {
        size = vec{
            std::max(size.width(), lines[i].width),
            size.height() + lines[i-1].descender + std::max(lines[i-1].lineGap, lines[i].lineGap) + lines[i].ascender
        };

        if (i == half_nr_lines) {
            if (odd_nr_lines) {
                // Take the base line of the middle line.
                base_line = size.height();
            } else {
                // Take the base line of the line-gap between the two middle lines.
                base_line = size.height() - lines[i].ascender - std::max(lines[i-1].lineGap, lines[i].lineGap) * 0.5f;
            }
        }
    }

    // Bottom of last line.
    size.height(size.height() + lines.back().descender + lines.back().lineGap);

    return {size, size.height() - base_line};
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
* @return size of the resulting text, shaped text.
*/
[[nodiscard]] static std::pair<vec,std::vector<AttributedGlyphLine>> shape_text(
    std::vector<AttributedGrapheme> text,
    HorizontalAlignment alignment,
    float maximum_width=std::numeric_limits<float>::max()) noexcept
{
    std::vector<AttributedGlyph> attributed_glyphs;

    // Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
    bidi_algorithm(text);

    // Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
    auto glyphs = graphemes_to_glyphs(text);

    // Morph attributed-glyphs using the Font's morph algorithm.
    morph_glyphs(glyphs);

    // Load metric for each attributed-glyph using many of the Font's tables.
    load_metrics_for_glyphs(glyphs);

    // Split the text up in lines, based on line-feeds and line-wrapping.
    auto lines = make_lines(glyphs, maximum_width);

    // Calculate actual size of the box, no smaller than the minimum_size.
    let [extent, base] = calculate_text_size(lines);

    // Align the text within the actual box size.
    position_glyphs(lines, alignment, extent);

    return {extent, lines};
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, HorizontalAlignment alignment, float maximum_width) noexcept
{
    ttauri_assume((alignment == HorizontalAlignment::Left) || (maximum_width < std::numeric_limits<float>::max()));
    std::tie(this->extent, this->lines) = shape_text(text, alignment, maximum_width);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), alignment, maximum_width) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, HorizontalAlignment alignment, float maximum_width) noexcept :
    ShapedText(translateString<gstring>(text), style, alignment, maximum_width) {}


[[nodiscard]] ShapedText::const_iterator ShapedText::find(ssize_t index) const noexcept
{
    return std::find_if(cbegin(), cend(), [=](let &x) {
        return x.containsLogicalIndex(index);
    });
}

[[nodiscard]] std::pair<vec,vec> ShapedText::carets(ssize_t position) const noexcept
{
    for (let &attr_glyph: *this) {
        if (position > attr_glyph.logicalIndex && position < attr_glyph.logicalIndex + attr_glyph.graphemeCount) {
            // The position is inside a ligature.
            // Place the cursor proportional inside the ligature, based on the font-metrics.
            let grapheme_index = attr_glyph.logicalIndex - position;
            let ligature_advance = attr_glyph.metrics.advanceForGrapheme(numeric_cast<int>(grapheme_index));

            let caret_position = attr_glyph.position + vec::point(ligature_advance);
            return {caret_position, vec{}};

        } else if ((position - 1) == attr_glyph.logicalIndex && attr_glyph.endOfParagraph) {
            // There is a non-linefeed glyph in the left side of the position, place the cursor
            // to the right of that glyph.
            let caret_position = attr_glyph.position + vec::point(attr_glyph.metrics.advance);
            return {caret_position, vec{}};

        }
    }

    // Either there was no glyph on the left of the position or it was a line-feed.
    // In both cases we need to know where the left-side of the glyph on the right.
    for (let &attr_glyph: *this) {
        if (position == attr_glyph.logicalIndex) {
            let caret_position = attr_glyph.position + vec::point();
            return {caret_position, vec{}};
        }
    }

    // If there are no glyphs on the left, or right, there must be no text at all.
    ttauri_assert(position == 0);
    // Let the caller figure out how to draw the cursor based on the selected character set.
    return {vec{}, vec{}};
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