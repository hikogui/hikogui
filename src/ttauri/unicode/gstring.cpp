// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "unicode_text_segmentation.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"

namespace tt::inline v1 {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    ttlet normalizedString = unicode_NFKC(
        rhs,
        unicode_normalization_mask::NFKD | unicode_normalization_mask::decompose_CR_LF | unicode_normalization_mask::compose_PS);

    auto r = tt::gstring{};
    auto breakState = tt::grapheme_break_state{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (breaks_grapheme(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += tt::grapheme::from_composed(cluster);
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (ssize(cluster) != 0) {
        r += tt::grapheme::from_composed(cluster);
    }
    return r;
}

} // namespace tt::inline v1
