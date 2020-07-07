// Copyright 2020 Pokitec
// All rights reserved.

#include "ttauri/text/gstring.hpp"
#include "ttauri/text/globals.hpp"
#include "ttauri/foundation/strings.hpp"

namespace tt {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    ttlet normalizedString = unicodeData->toNFC(rhs, true, true);

    auto r = tt::gstring{};
    auto breakState = tt::GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (unicodeData->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += tt::Grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (ssize(cluster) != 0) {
        r += tt::Grapheme{cluster};
    }
    return r;
}


}