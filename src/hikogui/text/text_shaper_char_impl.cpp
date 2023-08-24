// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_shaper_char.hpp"
#include "../font/font.hpp"
#include "../macros.hpp"

namespace hi::inline v1 {

[[nodiscard]] text_shaper_char::text_shaper_char(hi::grapheme const& grapheme, text_style const& style, float dpi_scale) noexcept
    :
    grapheme(grapheme),
    style(style),
    dpi_scale(dpi_scale),
    line_nr(std::numeric_limits<size_t>::max()),
    column_nr(std::numeric_limits<size_t>::max()),
    general_category(ucd_get_general_category(grapheme.starter()))
{
}

void text_shaper_char::set_glyph(hi::font_book::font_glyphs_type&& new_glyphs) noexcept
{
    glyphs = std::move(new_glyphs);
    hi_axiom_not_null(glyphs.font);
    scale = glyphs.get_font_metrics().round_scale(dpi_scale * style->size);
    metrics = scale * glyphs.get_starter_metrics();
}

void text_shaper_char::initialize_glyph(hi::font const& font) noexcept
{
    if (not glyph_is_initial) {
        set_glyph(find_glyph(font, grapheme));

        width = metrics.advance;
        glyph_is_initial = true;
    }
}

void text_shaper_char::initialize_glyph() noexcept
{
    return initialize_glyph(find_font(style->family_id, style->variant));
}

void text_shaper_char::replace_glyph(char32_t code_point) noexcept
{
    hi_axiom_not_null(glyphs.font);
    hilet& font = *glyphs.font;
    set_glyph(font_book::font_glyphs_type{font, font.find_glyph(code_point)});

    glyph_is_initial = false;
}

} // namespace hi::inline v1
