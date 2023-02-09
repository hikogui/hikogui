// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "font.hpp"
#include "true_type_font.hpp"

namespace hi::inline v1 {

[[nodiscard]] lean_vector<glyph_id> font::find_glyph(grapheme g) const
{
    // Create a glyph_ids object for a single grapheme.
    auto r = lean_vector<glyph_id>{};

    // First try composed normalization
    for (auto i = 0_uz; i != g.size(); ++i) {
        if (hilet glyph_id = find_glyph(g[i])) {
            r.push_back(glyph_id);
        } else {
            r.clear();
            break;
        }
    }

    if (r.empty()) {
        // Now try decomposed normalization
        for (hilet c : g.decomposed()) {
            if (hilet glyph_id = find_glyph(c)) {
                r.push_back(glyph_id);
            } else {
                r.clear();
                break;
            }
        }
    }

    return r;
}

} // namespace hi::inline v1
