// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "unicode_text_segmentation.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs, char32_t new_line_char) noexcept
{
    using enum unicode_normalization_mask;
    
    ttlet normalizedString = unicode_NFKC(rhs, NFKD | compose_CRLF | decompose_newline_to(new_line_char) | decompose_control);

    auto r = tt::gstring{};
    auto breakState = tt::grapheme_break_state{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (breaks_grapheme(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += tt::grapheme::from_composed(cluster);
                tt_axiom(r.back().valid());
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (ssize(cluster) != 0) {
        r += tt::grapheme::from_composed(cluster);
        tt_axiom(r.back().valid());
    }
    return r;
}

} // namespace tt::inline v1
