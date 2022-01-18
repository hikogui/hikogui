// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font.hpp"
#include "true_type_font.hpp"
#include "../resource_view.hpp"

namespace tt::inline v1 {

[[nodiscard]] glyph_ids font::find_glyph(grapheme g) const noexcept
{
    if (not loaded() and not unicode_mask.contains(g)) {
        // If the grapheme is not available in the font prevent font loading.
        // However if the font is loaded, then just look up the grapheme directly from the font.
        return {};
    }

    // Create a glyph_ids object for a single grapheme.
    auto r = glyph_ids(*this);
    r.set_num_graphemes(1);

    // First try composed normalization
    for (std::size_t i = 0; i != g.size(); ++i) {
        if (ttlet glyph_id = find_glyph(g[i])) {
            r += glyph_id;
        } else {
            r.clear();
            break;
        }
    }

    if (not r) {
        // First try decomposed normalization
        for (ttlet c : g.decomposed()) {
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

} // namespace tt::inline v1
