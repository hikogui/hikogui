// Copyright 2020 Pokitec
// All rights reserved.

#include "gstring.hpp"
#include "UnicodeData.hpp"
#include "../strings.hpp"
#include "../application.hpp"

namespace tt {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    ttlet normalizedString = application::global->unicodeData->toNFC(rhs, true, true);

    auto r = tt::gstring{};
    auto breakState = tt::GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (application::global->unicodeData->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += tt::Grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (std::ssize(cluster) != 0) {
        r += tt::Grapheme{cluster};
    }
    return r;
}


}