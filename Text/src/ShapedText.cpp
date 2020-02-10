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
    float vertical_advance = 0.0;
    float descender = 0.0;

    AttributedGlyphsLine(std::vector<AttributedGlyph>::iterator begin, std::vector<AttributedGlyph>::iterator end, float width, float vertical_advance, float descender) :
        begin(begin), end(end), width(width), vertical_advance(vertical_advance), descender(descender) {}
};

[[nodiscard]] static std::vector<AttributedGlyphsLine> make_lines(std::vector<AttributedGlyph> &glyphs, float maximum_width) noexcept
{
    std::vector<AttributedGlyphsLine> lines;

    auto start_of_line = glyphs.begin();
    auto end_of_last_word = start_of_line;
    float end_of_last_word_width = 0.0;
    auto previous = start_of_line;
    float previous_width = 0.0;
    float max_vertical_advance = 0.0;
    float max_decender = 0.0;
    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        let width = previous_width + (i->metrics.advance.x * i->style.size);
        max_vertical_advance = std::max(max_vertical_advance, 1.2f * (i->metrics.capHeight.y * i->style.size));
        max_decender = std::max(max_decender, i->metrics.descender.y * i->style.size);
        if (width > maximum_width) {
            // Break at end of previous word.
            lines.emplace_back(start_of_line, end_of_last_word, end_of_last_word_width, max_vertical_advance, max_decender);

            // Calculate the width remaining on the new line.
            previous_width = previous_width - end_of_last_word_width;

            // Reset counters to start of the new line.
            start_of_line = end_of_last_word;
            end_of_last_word = start_of_line;
            end_of_last_word_width = 0.0;
            max_vertical_advance = 0.0;
            max_decender = 0.0;
        }
        
        switch (i->grapheme.front()) {
        case ' ':
            end_of_last_word = i;
            end_of_last_word_width = previous_width;
            previous = i;
            previous_width = width;
            break;

        case '\n':
            lines.emplace_back(start_of_line, previous, previous_width, max_vertical_advance, max_decender);

            // Reset counters to start of the new line.
            start_of_line = i + 1;
            end_of_last_word = start_of_line;
            end_of_last_word_width = 0.0;
            previous = start_of_line;
            previous_width = 0.0;
            max_vertical_advance = 0.0;
            max_decender = 0.0;
            break;

        default:
            previous = i;
            previous_width = width;
        }
    }

    if (start_of_line != glyphs.end()) {
        lines.emplace_back(start_of_line, glyphs.end(), previous_width, max_vertical_advance, max_decender);
    }

    return lines;
}

[[nodiscard]] static extent2 calculate_text_size(std::vector<AttributedGlyphsLine> const &lines, extent2 size) noexcept
{
    if (ssize(lines) == 0) {
        return {0.0, 0.0};
    }

    auto height = 0.0f;
    for (let &line: lines) {
        size.width() = std::max(size.width(), line.width);
        height += line.vertical_advance;
    }

    height += lines.back().descender;

    size.height() = std::max(size.height(), height);
    return size;
}

static void position_glyphs(std::vector<AttributedGlyphsLine> &lines, extent2 size, extent2 minimum_size, Alignment alignment) noexcept
{
    auto position = glm::vec2{0.0f, 0.0f};

    if (alignment == VerticalAlignment::Base || size.height() >= minimum_size.height()) {
        position.y = 0.0f;
    } else if (alignment == VerticalAlignment::Top) {
        position.y = 0.0f;
    } else if (alignment == VerticalAlignment::Bottom) {
        position.y = minimum_size.height() - size.height();
    } else if (alignment == VerticalAlignment::Middle) {
        position.y = (minimum_size.height() - size.height()) * 0.5f;
    } else {
        no_default;
    }

    for (auto &line: lines) {
        position.y += line.vertical_advance;

        if (alignment == HorizontalAlignment::Left) {
            position.x = 0.0f;
        } else if (alignment == HorizontalAlignment::Right) {
            position.x = size.width() - line.width;
        } else if (alignment == HorizontalAlignment::Center) {
            position.x = (size.width() - line.width) * 0.5f;
        } else {
            no_default;
        }

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
    extent2 box_size;

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
    box_size = calculate_text_size(lines, minimum_size);

    // Align the text within the actual box size.
    position_glyphs(lines, box_size, minimum_size, alignment);

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

    ttauri_assume(Text_globals->font_book);
    let &font_book = *(Text_globals->font_book);

    auto previous_color = text.front().style.color;
    for (let &attr_glyph: text) {
        r += attr_glyph.get_path();
    }
    r.optimizeLayers();

    return r;
}


}