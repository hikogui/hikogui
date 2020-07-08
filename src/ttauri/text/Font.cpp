// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "Font.hpp"
#include "TrueTypeFont.hpp"
#include "../ResourceView.hpp"

namespace tt {

[[nodiscard]] FontGlyphIDs Font::find_glyph(Grapheme g) const noexcept
{
    FontGlyphIDs r;

    // First try composed normalization
    for (ssize_t i = 0; i != ssize(g); ++i) {
        if (ttlet glyph_id = find_glyph(g[i])) {
            r += glyph_id;
        } else {
            r.clear();
            break;
        }
    }

    if (!r) {
        // First try decomposed normalization
        for (ttlet c: g.NFD()) {
            if (ttlet glyph_id = find_glyph(c)) {
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

namespace tt {

template<>
std::unique_ptr<tt::Font> parseResource(URL const &location)
{
    if (location.extension() == "ttf") {
        auto view = location.loadView();

        try {
            return std::make_unique<tt::TrueTypeFont>(std::move(view));
        } catch (error &e) {
            e.set<url_tag>(location);
            throw;
        }

    } else {
        TTAURI_THROW(url_error("Unknown extension")
            .set<url_tag>(location)
        );
    }
}

}
