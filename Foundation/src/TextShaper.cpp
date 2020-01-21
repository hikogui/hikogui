// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/TextShaper.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

static void bidi_algorithm(std::vector<AttributedGrapheme> &text) noexcept
{

}

[[nodiscard]] static std::vector<AttributedGlyph> graphemes_to_glyphs(std::vector<AttributedGrapheme> const &text) noexcept
{
    std::vector<AttributedGlyph> glyphs;
    glyphs.reserve(size(text));

    let &font_book = *(Foundation_globals->font_book);

    for (let &ag: text) {
        auto font_id = font_book.find_font(ag.style.family_id, ag.style.font_variant);
        auto glyph = font_book.find_glyph(font_id, ag.g);
        glyphs.emplace_back(glyph);
    }

    return glyphs;
}

static void morph_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{

}

static void advance_glyphs(std::vector<AttributedGlyph> &glyphs) noexcept
{

}

static void line_break_glyphs(std::vector<AttributedGlyph> &glyphs, float width) noexcept
{

}

[[nodiscard]] static extent2 calculate_text_size(std::vector<AttributedGlyph> &glyphs, extent2 minimum_size) noexcept
{
    return {};
}

static void align_glyphs(std::vector<AttributedGlyph> &glyphs, extent2 maximum_size, Alignment alignment) noexcept
{

}


[[nodiscard]] std::pair<extent2,std::vector<AttributedGlyph>> shape_text(std::vector<AttributedGrapheme> text, Alignment alignment, extent2 minimum_size, extent2 maximum_size) noexcept
{
    std::vector<AttributedGlyph> attributed_glyphs;
    extent2 box_size;

    // Put graphemes in left-to-right display order using the UnicodeData's bidi_algorithm.
    bidi_algorithm(text);

    // Convert attributed-graphemes into attributes-glyphs using FontBook's find_glyph algorithm.
    auto glyphs = graphemes_to_glyphs(text);

    // Morph attributed-glyphs using the Font's morph algorithm.
    morph_glyphs(glyphs);

    // Calculate advance for each attributed-glyph using the Font's advance and kern algorithms.
    advance_glyphs(glyphs);

    // Add line-breaks to the text to fit within the maximum-width.
    line_break_glyphs(glyphs, maximum_size.width());

    // Calculate actual size of the box, no smaller than the minimum_size.
    box_size = calculate_text_size(glyphs, minimum_size);

    // Align the text within the actual box size.
    align_glyphs(glyphs, box_size, alignment);

    return {box_size, glyphs};
}

}