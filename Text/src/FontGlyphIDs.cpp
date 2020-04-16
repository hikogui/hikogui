// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/AttributedGlyph.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/Path.hpp"

namespace TTauri::Text {

[[nodiscard]] Path FontGlyphIDs::get_path() const noexcept {
    Path r;

    let &font = fontBook->get_font(font_id());

    for (ssize_t i = 0; i < ssize(*this); i++) {
        let glyph_id = (*this)[i];

        Path glyph_path;
        if (!font.loadGlyph(glyph_id, glyph_path)) {
            LOG_ERROR("Could not find glyph {} in font {} - {}", static_cast<int>(glyph_id), font.description.family_name, font.description.sub_family_name);
        }

        r += glyph_path;
    }

    return r;
}

}