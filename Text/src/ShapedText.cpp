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
        r.emplace_back(grapheme, index++, style);
    }

    return r;
}

static void bidi_algorithm(std::vector<AttributedGrapheme> &text) noexcept
{

}

[[nodiscard]] static std::vector<AttributedGlyph> graphemes_to_glyphs(std::vector<AttributedGrapheme> const &text) noexcept
{
    std::vector<AttributedGlyph> glyphs;
    glyphs.reserve(size(text));

    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    for (let &ag: text) {
        let font_id = font_book.find_font(ag.style.family_id, ag.style.variant);
        glyphs.emplace_back(ag, font_book.find_glyph(font_id, ag.grapheme));
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

        // XXX merge the bounding box of combining glyphs in the metrics.
    }
}

struct AttributedGlyphsLine {
    std::vector<AttributedGlyph>::iterator begin;
    std::vector<AttributedGlyph>::iterator end;

    float width = 0.0;
    float ascender = 0.0;
    float descender = 0.0;
    float lineGap = 0.0;
    float xHeight = 0.0;

    AttributedGlyphsLine(std::vector<AttributedGlyph>::iterator begin, std::vector<AttributedGlyph>::iterator end, float width, float ascender, float descender, float lineGap, float xHeight) :
        begin(begin), end(end), width(width), ascender(ascender), descender(descender), lineGap(lineGap), xHeight(xHeight) {}
};

[[nodiscard]] static std::vector<AttributedGlyphsLine> make_lines(std::vector<AttributedGlyph> &glyphs, float maximum_width) noexcept
{
    std::vector<AttributedGlyphsLine> lines;

    auto start_of_line = glyphs.begin();
    auto end_of_last_word = start_of_line;
    float end_of_last_word_width = 0.0;
    auto previous = start_of_line;
    float previous_width = 0.0;
    float max_ascender = 0.0;
    float max_descender = 0.0;
    float max_line_gap = 0.0;
    float max_x_height = 0.0;
    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        let width = previous_width + (i->metrics.advance.x * i->style.size);
        max_ascender = std::max(max_ascender, i->metrics.ascender * i->style.size);
        max_descender = std::max(max_descender, -(i->metrics.descender * i->style.size));
        max_line_gap = std::max(max_line_gap, i->metrics.lineGap * i->style.size);
        max_x_height = std::max(max_x_height, i->metrics.xHeight * i->style.size);
        if (width > maximum_width) {
            // Break at end of previous word.
            lines.emplace_back(start_of_line, end_of_last_word, end_of_last_word_width, max_ascender, max_descender, max_line_gap, max_x_height);

            // Calculate the width remaining on the new line.
            previous_width = previous_width - end_of_last_word_width;

            // Reset counters to start of the new line.
            start_of_line = end_of_last_word;
            end_of_last_word = start_of_line;
            end_of_last_word_width = 0.0;
            max_ascender = 0.0;
            max_descender = 0.0;
            max_line_gap = 0.0;
            max_x_height = 0.0;
        }
        
        switch (i->grapheme.front()) {
        case ' ':
            end_of_last_word = previous + 1;
            end_of_last_word_width = previous_width;
            previous = i;
            previous_width = width;
            break;

        case '\n':
            lines.emplace_back(start_of_line, previous + 1, previous_width, max_ascender, max_descender, max_line_gap, max_x_height);

            // Reset counters to start of the new line.
            start_of_line = i + 1;
            end_of_last_word = start_of_line;
            end_of_last_word_width = 0.0;
            previous = start_of_line;
            previous_width = 0.0;
            max_ascender = 0.0;
            max_descender = 0.0;
            max_line_gap = 0.0;
            max_x_height = 0.0;
            break;

        default:
            previous = i;
            previous_width = width;
        }
    }

    if (start_of_line != glyphs.end()) {
        lines.emplace_back(start_of_line, glyphs.end(), previous_width, max_ascender, max_descender, max_line_gap, max_x_height);
    }

    return lines;
}

[[nodiscard]] static std::pair<extent2,float> calculate_text_size(std::vector<AttributedGlyphsLine> const &lines) noexcept
{
    auto size = extent2{0.0f, 0.0f};

    if (ssize(lines) == 0) {
        return {size, 0.0f};
    }

    // Top of first line.
    size.width() = lines.front().width;
    size.height() = lines.front().lineGap + lines.front().ascender;
    auto optical_middle = size.height() - lines.front().xHeight * 0.5f;

    auto nr_lines = ssize(lines);
    auto half_nr_lines = nr_lines / 2;
    auto odd_nr_lines = nr_lines % 2 == 1;
    for (ssize_t i = 1; i != nr_lines; ++i) {
        size.width() = std::max(size.width(), lines[i].width);
        size.height() += lines[i-1].descender + std::max(lines[i-1].lineGap, lines[i].lineGap) + lines[i].ascender;

        if (i == half_nr_lines) {
            if (odd_nr_lines) {
                optical_middle = size.height() - lines[i].xHeight * 0.5f;
            } else {
                optical_middle = size.height() - lines[i].ascender - std::max(lines[i-1].lineGap, lines[i].lineGap) * 0.5f;
            }
        }
    }

    // Bottom of last line.
    size.height() += lines.back().descender + lines.back().lineGap;

    return {size, size.height() - optical_middle};
}

static void position_glyphs(std::vector<AttributedGlyphsLine> &lines, extent2 size, extent2 minimum_size, Alignment alignment, float optical_middle) noexcept
{
    // Calculate where the top edge of the text should be drawn.
    float y = 0.0f;
    if (size.height() >= minimum_size.height()) {
        y = size.height();
    } else if (alignment == VerticalAlignment::Base) {
        // Center text based on the total height.
        y = (minimum_size.height() * 0.5f - optical_middle) + size.height();
    } else if (alignment == VerticalAlignment::Top) {
        y = minimum_size.height();
    } else if (alignment == VerticalAlignment::Bottom) {
        y = size.height();
    } else if (alignment == VerticalAlignment::Middle) {
        // Center text based on the total height.
        y = minimum_size.height() - (minimum_size.height() - size.height()) * 0.5f;
    } else {
        no_default;
    }

    
    for (ssize_t i = 0; i != ssize(lines); ++i) {
        let &line = lines[i];
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
            x = std::max(size.width(), minimum_size.width()) - line.width;
        } else if (alignment == HorizontalAlignment::Center) {
            x = (std::max(size.width(), minimum_size.width()) - line.width) * 0.5f;
        } else {
            no_default;
        }

        auto position = glm::vec2{x, y};
        for (auto i = line.begin; i != line.end; ++i) {
            i->transform = T2D(position, i->style.size);
            position += i->metrics.advance * i->style.size;
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
*  - Calculate actual size of the box, no smaller than the minimum_size.
*  - Align the text within the actual box size.
*
* @param text The text to be shaped.
* @param max_width Maximum width that the text should flow into.
* @param alignment How the text should be aligned in the box.
* @return size of the resulting text, shaped text.
*/
[[nodiscard]] static std::pair<extent2,std::vector<AttributedGlyph>> shape_text(std::vector<AttributedGrapheme> text, Alignment alignment, extent2 minimum_size, extent2 maximum_size) noexcept
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

    auto lines = make_lines(glyphs, maximum_size.width());

    // Calculate actual size of the box, no smaller than the minimum_size.
    let [box_size, optical_middle] = calculate_text_size(lines);

    // Align the text within the actual box size.
    position_glyphs(lines, box_size, minimum_size, alignment, optical_middle);

    return {box_size, glyphs};
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept
{
    std::tie(this->box_size, this->text) = shape_text(text, alignment, minimum_size, maximum_size);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), alignment, minimum_size, maximum_size) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, Alignment const &alignment, extent2 const &minimum_size, extent2 const &maximum_size) noexcept :
    ShapedText(translateString<gstring>(text), style, alignment, minimum_size, maximum_size) {}

[[nodiscard]] Path ShapedText::get_path() const noexcept
{
    Path r;

    if (ssize(text) == 0) {
        return r;
    }

    for (let &attr_glyph: text) {
        r += attr_glyph.get_path();
    }
    r.optimizeLayers();

    return r;
}


}