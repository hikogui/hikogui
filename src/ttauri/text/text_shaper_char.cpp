// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "font_book.hpp"

namespace tt::inline v1 {

[[nodiscard]] text_shaper_char::text_shaper_char(tt::grapheme const &grapheme, text_style const &style) noexcept :
    grapheme(grapheme), style(style), description(&unicode_description_find(grapheme[0]))
{
}

void text_shaper_char::set_glyph(tt::glyph_ids &&new_glyph) noexcept
{
    glyph = std::move(new_glyph);
    auto glyph_metrics = tt::glyph_metrics{};
    if (glyph.font().load_glyph_metrics(glyph[0], glyph_metrics)) {
        scale = glyph.font().metrics.round_scale(style.scaled_size());
        metrics = scale * glyph_metrics;
    } else {
        // Failed to load metrics, due to corrupt font file.
        metrics = {};
    }
}

void text_shaper_char::initialize_glyph(tt::font_book &font_book, tt::font const &font) noexcept
{
    if (not glyph_is_initial) {
        set_glyph(font_book.find_glyph(font, grapheme));

        width = metrics.advance.x();
        glyph_is_initial = true;
    }
}

void text_shaper_char::initialize_glyph(tt::font_book &font_book) noexcept
{
    return initialize_glyph(font_book, font_book.find_font(style.family_id, style.variant));
}

void text_shaper_char::replace_glyph(char32_t code_point) noexcept
{
    ttlet &font = glyph.font();
    set_glyph(font.find_glyph(tt::grapheme{code_point}));

    glyph_is_initial = false;
}

}