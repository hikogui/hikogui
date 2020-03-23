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
    std::vector<AttributedGlyph>::iterator _begin;
    std::vector<AttributedGlyph>::iterator _end;

    float width = 0.0;
    float ascender = 0.0;
    float descender = 0.0;
    float lineGap = 0.0;
    float xHeight = 0.0;

    AttributedGlyphsLine(std::vector<AttributedGlyph>::iterator begin, std::vector<AttributedGlyph>::iterator end) :
        _begin(begin), _end(end), width(0.0f), ascender(0.0f), descender(0.0f), lineGap(0.0f), xHeight(0.0f) {}

    auto begin() const noexcept { return _begin; }
    auto end() const noexcept { return _end; }
};

[[nodiscard]] static std::vector<AttributedGlyphsLine> make_lines(std::vector<AttributedGlyph> &glyphs, float maximum_width) noexcept
{
    std::vector<AttributedGlyphsLine> lines;

    float width = 0.0f;
    auto start_of_line = glyphs.begin();
    auto end_of_word = glyphs.begin();

    for (auto i = glyphs.begin(); i != glyphs.end(); ++i) {
        switch (i->grapheme.front()) {
        case '\n':
            lines.emplace_back(start_of_line, i);
            width = 0.0f;
            start_of_line = i + 1;
            end_of_word = i + 1;
            continue;

        case ' ':
            end_of_word = i;
            break;

        default:;
        }

        width += (i->metrics.advance.x() * i->style.size);

        if ((width > maximum_width) && (start_of_line != end_of_word)) {
            // Line is to long, and exists of at least a full word.
            lines.emplace_back(start_of_line, end_of_word);

            // Skip back in the for-loop.
            // The position of the white-space, the white-space will be skipped over by the end-of-for-loop.
            i = end_of_word;
            start_of_line = i + 1;
            end_of_word = i + 1;
        }
    }

    if (start_of_line != glyphs.end()) {
        lines.emplace_back(start_of_line, glyphs.end());
    }

    return lines;
}

void calculate_line_sizes(std::vector<AttributedGlyphsLine> &lines) noexcept
{
    for (auto &line: lines) {
        for (let &glyph: line) {
            let font_size = glyph.style.size;
            let &metrics = glyph.metrics;

            line.width += metrics.advance.x() * font_size;
            line.ascender = std::max(line.ascender, metrics.ascender * font_size);
            line.descender = std::max(line.descender, -metrics.descender * font_size);
            line.lineGap = std::max(line.lineGap, metrics.lineGap * font_size);
            line.xHeight = std::max(line.xHeight, metrics.xHeight * font_size);
        }
    }
}

/** Calculate the size of the text.
 * @return The extent of the text and the base line position of the middle line.
 */
[[nodiscard]] static std::pair<vec,float> calculate_text_size(std::vector<AttributedGlyphsLine> const &lines) noexcept
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

/**
 */
static void position_glyphs(std::vector<AttributedGlyphsLine> &lines, vec text_extent, float text_base, vec box_extent, Alignment alignment) noexcept
{
    // Calculate where text should be drawn compared to the text.
    float y = 0.0f;
    if (alignment == VerticalAlignment::Base) {
        // Center text based on the total height.
        y = box_extent.height() * 0.5f - text_base;

    } else if (alignment == VerticalAlignment::Top) {
        y = box_extent.height() - text_extent.height();

    } else if (alignment == VerticalAlignment::Bottom) {
        y = 0.0f;

    } else if (alignment == VerticalAlignment::Middle) {
        // Center text based on the total height.
        y = box_extent.height() * 0.5f - text_extent.height() * 0.5f;

    } else {
        no_default;
    }

    // Draw lines from the top-to-down.
    y += text_extent.height();
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
            x = box_extent.width() - line.width;
        } else if (alignment == HorizontalAlignment::Center) {
            x = box_extent.width() * 0.5f - line.width * 0.5f;
        } else {
            no_default;
        }

        auto position = vec(x, y);
        for (auto &glyph: line) {
            glyph.transform = mat::T(position) * mat::S(glyph.style.size, glyph.style.size);
            position += vec{glyph.style.size, glyph.style.size, 1.0} * glyph.metrics.advance;
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
* @param max_width Maximum width that the text should flow into.
* @param alignment How the text should be aligned in the box.
* @return size of the resulting text, shaped text.
*/
[[nodiscard]] static std::pair<vec,std::vector<AttributedGlyph>> shape_text(std::vector<AttributedGrapheme> text, vec extent, Alignment alignment, bool wrap) noexcept
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
    auto lines = make_lines(glyphs, wrap ? extent.width() : std::numeric_limits<float>::max());

    // Calculate sizes of each line.
    calculate_line_sizes(lines);

    // Calculate actual size of the box, no smaller than the minimum_size.
    let [text_extent, text_base] = calculate_text_size(lines);

    // Align the text within the actual box size.
    position_glyphs(lines, text_extent, text_base, extent, alignment);

    return {text_extent, glyphs};
}


ShapedText::ShapedText(std::vector<AttributedGrapheme> const &text, vec extent, Alignment alignment, bool wrap) noexcept :
    extent(extent), alignment(alignment), wrap(wrap)
{
    std::tie(this->text_extent, this->text) = shape_text(text, extent, alignment, wrap);
}

ShapedText::ShapedText(gstring const &text, TextStyle const &style, vec extent, Alignment alignment, bool wrap) noexcept :
    ShapedText(makeAttributedGraphemeVector(text, style), extent, alignment, wrap) {}

ShapedText::ShapedText(std::string const &text, TextStyle const &style, vec extent, Alignment alignment, bool wrap) noexcept :
    ShapedText(translateString<gstring>(text), style, extent, alignment, wrap) {}

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