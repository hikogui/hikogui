// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Text/FontBook.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/Path.hpp"

namespace tt {

AttributedGlyph::AttributedGlyph(AttributedGrapheme const &attr_grapheme, AttributedGlyph const *next_attr_glyph) noexcept :
    logicalIndex(attr_grapheme.logicalIndex),
    graphemeCount(1),
    charClass(attr_grapheme.charClass),
    style(attr_grapheme.style)
{
    // Get the font_id that matches the requested style.
    ttlet style_font_id = fontBook->find_font(attr_grapheme.style.family_id, attr_grapheme.style.variant);

    // The end-of-paragraph is represented by a space glyph, which is usefull for
    // producing a correct cursor at an empty line at the end of a paragraph.
    ttlet g = (attr_grapheme.grapheme == '\n') ? Grapheme{0} : attr_grapheme.grapheme;

    glyphs = fontBook->find_glyph(style_font_id, g);

    // Get the font_id that contained the glyph, this may be different if the glyph was not in the
    // style_font and was found in a fallback font. This may happend for text in a different language
    // then the language of the requested font.
    ttlet actual_font_id = glyphs.font_id();

    // Load the metrics for this attributed glyph.
    ttlet font = &(fontBook->get_font(actual_font_id));
    ttauri_assume(font != nullptr);

    // Get the metrics of the main glyph.
    ttlet this_glyph = glyphs.front();

    // If the next glyph is of the same font, then use it for kerning reasons.
    ttlet next_glyph = (next_attr_glyph && next_attr_glyph->glyphs.font_id() == actual_font_id) ?
        next_attr_glyph->glyphs.front() :
        GlyphID{};

    if (!font->loadGlyphMetrics(this_glyph, metrics, next_glyph)) {
        LOG_ERROR("Could not load metrics for glyph {} in font {} - {}", static_cast<int>(this_glyph), font->description.family_name, font->description.sub_family_name);
        // failed to load metrics. Switch to glyph zero and load again.
        glyphs.clear();
        glyphs.set_font_id(style_font_id);
        glyphs += GlyphID{0};
        if (!font->loadGlyphMetrics(glyphs.front(), metrics)) {
            // Using null-metrics when even the null-glyph can not be found.
            LOG_ERROR("Could not load metrics for null-glyph in font {} - {}", font->description.family_name, font->description.sub_family_name);
        }
    }

    // Scale the metrics according to font-size of this glyph.
    metrics.scale(style.scaled_size());
}

[[nodiscard]] Path AttributedGlyph::get_path() const noexcept
{
    ttlet M = mat::T(position) * mat::S(style.scaled_size(), style.scaled_size());

    auto [glyph_path, glyph_bounding_box] = glyphs.getPathAndBoundingBox();
    auto transformed_glyph_path = M * glyph_path;
    transformed_glyph_path.closeLayer(style.color);
    return transformed_glyph_path;
}

}