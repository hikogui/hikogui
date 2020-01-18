// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/TrueTypeFont.hpp"
#include "TTauri/Foundation/ResourceView.hpp"

namespace TTauri {

[[nodiscard]] FontGlyphIDs Font::getGlyph(grapheme g) const noexcept
{
    FontGlyphIDs r;

    // First try composed normalization
    std::vector<Path> graphemeGlyphs;
    for (let codePoint: g.NFC()) {
        if (let glyphIndex = getGlyph(codePoint)) {
            r += glyphIndex;
        } else {
            r.clear();
            break;
        }
    }

    if (!r) {
        // First try decomposed normalization
        for (let codePoint: g.NFD()) {
            if (let glyphIndex = getGlyph(codePoint)) {
                r += glyphIndex;
            } else {
                r.clear();
                break;
            }
        }
    }

    return r;
}

template<>
std::unique_ptr<Font> parseResource(URL const &location)
{
    if (location.extension() == "ttf") {
        auto view = ResourceView::loadView(location);

        try {
            auto font = std::make_unique<TrueTypeFont>(std::move(view));
            return font;
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
