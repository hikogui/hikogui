// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/Path.hpp"

namespace TTauri::Text {

[[nodiscard]] std::pair<Path,aarect> FontGlyphIDs::getPathAndBoundingBox() const noexcept {
    Path path;
    auto boundingBox = aarect{};

    let &font = fontBook->get_font(font_id());
    for (ssize_t i = 0; i < ssize(*this); i++) {
        let glyph_id = (*this)[i];

        Path glyph_path;
        if (!font.loadGlyph(glyph_id, glyph_path)) {
            LOG_ERROR("Could not load glyph {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }
        path += glyph_path;

        GlyphMetrics glyph_metrics;
        if (!font.loadGlyphMetrics(glyph_id, glyph_metrics)) {
            LOG_ERROR("Could not load glyph-metrics {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }

        if (i == 0) {
            boundingBox = glyph_metrics.boundingBox;
        } else {
            boundingBox |= glyph_metrics.boundingBox;
        }
    }

    return {path, boundingBox};
}

[[nodiscard]] aarect FontGlyphIDs::getBoundingBox() const noexcept {
    Path path;
    auto boundingBox = aarect{};

    let &font = fontBook->get_font(font_id());
    for (ssize_t i = 0; i < ssize(*this); i++) {
        let glyph_id = (*this)[i];

        GlyphMetrics glyph_metrics;
        if (!font.loadGlyphMetrics(glyph_id, glyph_metrics)) {
            LOG_ERROR("Could not load glyph-metrics {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }

        if (i == 0) {
            boundingBox = glyph_metrics.boundingBox;
        } else {
            boundingBox |= glyph_metrics.boundingBox;
        }
    }

    return boundingBox;
}

}