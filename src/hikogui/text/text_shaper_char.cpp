// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "font_book.hpp"

namespace hi::inline v1 {

[[nodiscard]] text_shaper_char::text_shaper_char(hi::grapheme const &grapheme, text_style const &style, float dpi_scale) noexcept :
    grapheme(grapheme),
    style(style),
    dpi_scale(dpi_scale),
    line_nr(std::numeric_limits<size_t>::max()),
    column_nr(std::numeric_limits<size_t>::max()),
    description(&unicode_description::find(grapheme[0]))
{
}

void text_shaper_char::set_glyph(hi::glyph_ids &&new_glyph) noexcept
{
    glyph = std::move(new_glyph);
    auto glyph_metrics = hi::glyph_metrics{};
    if (glyph.font().load_glyph_metrics(glyph[0], glyph_metrics)) {
        scale = glyph.font().metrics.round_scale(dpi_scale * style->size);
        metrics = scale * glyph_metrics;
    } else {
        // Failed to load metrics, due to corrupt font file.
        metrics = {};
    }
}

void text_shaper_char::initialize_glyph(hi::font_book &font_book, hi::font const &font) noexcept
{
    if (not glyph_is_initial) {
        set_glyph(font_book.find_glyph(font, grapheme));

        width = metrics.advance.x();
        glyph_is_initial = true;
    }
}

void text_shaper_char::initialize_glyph(hi::font_book &font_book) noexcept
{
    return initialize_glyph(font_book, font_book.find_font(style->family_id, style->variant));
}

void text_shaper_char::replace_glyph(char32_t code_point) noexcept
{
    hilet &font = glyph.font();
    set_glyph(font.find_glyph(hi::grapheme{code_point}));

    glyph_is_initial = false;
}

} // namespace hi::inline v1