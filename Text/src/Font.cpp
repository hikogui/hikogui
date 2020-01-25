// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/Font.hpp"
#include "TTauri/Text/TrueTypeFont.hpp"
#include "TTauri/Foundation/ResourceView.hpp"

namespace TTauri::Text {

[[nodiscard]] FontGlyphIDs Font::find_glyph(Grapheme g) const noexcept
{
    FontGlyphIDs r;

    // First try composed normalization
    for (ssize_t i = 0; i != ssize(g); ++i) {
        if (let glyph_id = find_glyph(g[i])) {
            r += glyph_id;
        } else {
            r.clear();
            break;
        }
    }

    if (!r) {
        // First try decomposed normalization
        for (let c: g.NFD()) {
            if (let glyph_id = find_glyph(c)) {
                r += glyph_id;
            } else {
                r.clear();
                break;
            }
        }
    }

    return r;
}

}

namespace TTauri {

template<>
std::unique_ptr<TTauri::Text::Font> parseResource(URL const &location)
{
    if (location.extension() == "ttf") {
        auto view = ResourceView::loadView(location);

        try {
            return std::make_unique<TTauri::Text::TrueTypeFont>(std::move(view));
        } catch (error &e) {
            e.set<"url"_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<"url"_tag>(location)
        );
    }
}

}
