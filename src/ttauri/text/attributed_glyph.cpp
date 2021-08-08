// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "attributed_glyph.hpp"
#include "font_book.hpp"
#include "../logger.hpp"
#include "../graphic_path.hpp"

namespace tt {

attributed_glyph::attributed_glyph(
    tt::font_book const &font_book,
    attributed_grapheme const &attr_grapheme,
    attributed_glyph const *next_attr_glyph) noexcept :
    logicalIndex(attr_grapheme.logicalIndex),
    graphemeCount(1),
    general_category(attr_grapheme.general_category),
    style(attr_grapheme.style)
{
    // Get the font_id that matches the requested style.
    ttlet &style_font = font_book.find_font(attr_grapheme.style.family_id, attr_grapheme.style.variant);

    // The end-of-paragraph is represented by a space glyph, which is useful for
    // producing a correct cursor at an empty line at the end of a paragraph.
    ttlet g = (attr_grapheme.grapheme == '\n') ? grapheme{0} : attr_grapheme.grapheme;

    // The glyph returned here may be of a different font from the given style font.
    glyphs = font_book.find_glyph(style_font, g);

    // Get the metrics of the main glyph.
    ttlet this_glyph = glyphs.front();

    // If the next glyph is of the same font, then use it for kerning reasons.
    ttlet next_glyph = (next_attr_glyph && &next_attr_glyph->glyphs.font() == &glyphs.font()) ?
        next_attr_glyph->glyphs.front() :
        glyph_id{};

    if (not glyphs.font().load_glyph_metrics(this_glyph, metrics, next_glyph)) {
        tt_log_error(
            "Could not load metrics for glyph {} in font {} - {}",
            static_cast<int>(this_glyph),
            glyphs.font().family_name,
            glyphs.font().sub_family_name);

        // failed to load metrics. Switch to glyph zero and load again.
        glyphs.clear();
        glyphs.set_font(style_font);
        glyphs += glyph_id{0};
        if (not glyphs.font().load_glyph_metrics(glyphs.front(), metrics)) {
            // Using null-metrics when even the null-glyph can not be found.
            tt_log_error(
                "Could not load metrics for null-glyph in font {} - {}",
                glyphs.font().family_name,
                glyphs.font().sub_family_name);
        }
    }

    // Scale the metrics according to font-size of this glyph.
    metrics.scale(style.scaled_size());
}

[[nodiscard]] graphic_path attributed_glyph::get_path() const noexcept
{
    ttlet M = translate2(position) * scale2(style.scaled_size(), style.scaled_size());

    auto [glyph_path, glyph_bounding_box] = glyphs.get_path_and_bounding_box();
    auto transformed_glyph_path = M * glyph_path;
    transformed_glyph_path.closeLayer(style.color);
    return transformed_glyph_path;
}

}
