// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "unicode_text_segmentation.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"

namespace hi::inline v1 {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs, char32_t new_line_char) noexcept
{
    using enum unicode_normalization_mask;
    
    hilet normalizedString = unicode_NFKC(rhs, NFKD | compose_CRLF | decompose_newline_to(new_line_char) | decompose_control);

    auto r = gstring{};
    auto breakState = grapheme_break_state{};
    auto cluster = std::u32string{};

    for (hilet codePoint : normalizedString) {
        if (breaks_grapheme(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += grapheme(composed_t{}, cluster);
                hi_axiom(r.back().valid());
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (ssize(cluster) != 0) {
        r += grapheme(composed_t{}, cluster);
        hi_assert(r.back().valid());
    }
    return r;
}

} // namespace hi::inline v1
