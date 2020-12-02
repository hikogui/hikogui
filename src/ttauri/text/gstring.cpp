// Copyright 2020 Pokitec
// All rights reserved.

#include "gstring.hpp"
#include "unicode_data.hpp"
#include "../strings.hpp"
#include "../application.hpp"

namespace tt {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    ttlet normalizedString = unicode_data::global->toNFC(rhs, true, true);

    auto r = tt::gstring{};
    auto breakState = tt::GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (ttlet codePoint : normalizedString) {
        if (unicode_data::global->checkGraphemeBreak(codePoint, breakState)) {
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