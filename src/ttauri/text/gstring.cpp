// Copyright 2020 Pokitec
// All rights reserved.

#include "gstring.hpp"
#include "unicode_text_segmentation.hpp"
#include "unicode_normalization.hpp"
#include "../strings.hpp"
#include "../application.hpp"

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