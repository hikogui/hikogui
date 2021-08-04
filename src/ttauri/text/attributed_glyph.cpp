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
    ttlet style_font_id = font_book.find_font(attr_grapheme.style.family_id, attr_grapheme.style.variant);

    // The end-of-paragraph is represented by a space glyph, which is usefull for
    // producing a correct cursor at an empty line at the end of a paragraph.
    ttlet g = (attr_grapheme.grapheme == '\n') ? grapheme{0} : attr_grapheme.grapheme;

    glyphs = font_book.find_glyph(style_font_id, g);

    // Get the font_id that contained the glyph, this may be different if the glyph was not in the
    // style_font and was found in a fallback font. This may happend for text in a different language
    // then the language of the requested font.
    ttlet actual_font_id = glyphs.font_id();

    // Load the metrics for this attributed glyph.
    ttlet font = &(font_book.get_font(actual_font_id));
    tt_axiom(font != nullptr);

    // Get the metrics of the main glyph.
    ttlet this_glyph = glyphs.front();

    // If the next glyph is of the same font, then use it for kerning reasons.
    ttlet next_glyph = (next_attr_glyph && next_attr_glyph->glyphs.font_id() == actual_font_id) ?
        next_attr_glyph->glyphs.front() :
        glyph_id{};

    if (!font->loadglyph_metrics(this_glyph, metrics, next_glyph)) {
        tt_log_error("Could not load metrics for glyph {} in font {} - {}", static_cast<int>(this_glyph), font->description.family_name, font->description.sub_family_name);
        // failed to load metrics. Switch to glyph zero and load again.
        glyphs.clear();
        glyphs.set_font_id(style_font_id);
        glyphs += glyph_id{0};
        if (!font->loadglyph_metrics(glyphs.front(), metrics)) {
            // Using null-metrics when even the null-glyph can not be found.
            tt_log_error("Could not load metrics for null-glyph in font {} - {}", font->description.family_name, font->description.sub_family_name);
        }
    }

    // Scale the metrics according to font-size of this glyph.
    metrics.scale(style.scaled_size());
}

[[nodiscard]] graphic_path attributed_glyph::get_path(tt::font_book const &font_book) const noexcept
{
    ttlet M = translate2(position) * scale2(style.scaled_size(), style.scaled_size());

    auto [glyph_path, glyph_bounding_box] = glyphs.get_path_and_bounding_box(font_book);
    auto transformed_glyph_path = M * glyph_path;
    transformed_glyph_path.closeLayer(style.color);
    return transformed_glyph_path;
}

}
