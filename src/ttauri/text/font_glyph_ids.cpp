// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "attributed_glyph.hpp"
#include "font_book.hpp"
#include "../application.hpp"
#include "../Path.hpp"
#include "../logger.hpp"

namespace tt {

[[nodiscard]] std::pair<Path,aarect> font_glyph_ids::getPathAndBoundingBox() const noexcept {
    Path path;
    auto boundingBox = aarect{};

    ttlet &font = font_book::global->get_font(font_id());
    for (ssize_t i = 0; i < std::ssize(*this); i++) {
        ttlet glyph_id = (*this)[i];

        Path glyph_path;
        if (!font.loadGlyph(glyph_id, glyph_path)) {
            tt_log_error("Could not load glyph {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }
        path += glyph_path;

        glyph_metrics glyph_metrics;
        if (!font.loadglyph_metrics(glyph_id, glyph_metrics)) {
            tt_log_error("Could not load glyph-metrics {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }

        if (i == 0) {
            boundingBox = glyph_metrics.boundingBox;
        } else {
            boundingBox |= glyph_metrics.boundingBox;
        }
    }

    return {path, boundingBox};
}

[[nodiscard]] aarect font_glyph_ids::getBoundingBox() const noexcept {
    Path path;
    auto boundingBox = aarect{};

    ttlet &font = font_book::global->get_font(font_id());
    for (ssize_t i = 0; i < std::ssize(*this); i++) {
        ttlet glyph_id = (*this)[i];

        glyph_metrics glyph_metrics;
        if (!font.loadglyph_metrics(glyph_id, glyph_metrics)) {
            tt_log_error("Could not load glyph-metrics {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
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