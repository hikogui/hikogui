// Copyright 2019, 2020 Pokitec
// All rights reserved.

#include "font.hpp"
#include "TrueTypefont.hpp"
#include "../resource_view.hpp"

namespace tt {

[[nodiscard]] font_glyph_ids font::find_glyph(Grapheme g) const noexcept
{
    font_glyph_ids r;

    // First try composed normalization
    for (ssize_t i = 0; i != std::ssize(g); ++i) {
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
