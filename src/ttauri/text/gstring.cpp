// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "gstring.hpp"
#include "unicode_text_segmentation.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"

namespace tt {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    ttlet normalizedString = unicode_NFC(rhs, true, true, true);

    auto r = tt::gstring{};
    auto breakState = tt::grapheme_break_state{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (breaks_grapheme(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += tt::grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (std::ssize(cluster) != 0) {
        r += tt::grapheme{cluster};
    }
    return r;
}


}