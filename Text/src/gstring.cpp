// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/gstring.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/strings.hpp"

namespace TTauri::Text {

[[nodiscard]] gstring to_gstring(std::u32string_view rhs) noexcept
{
    let normalizedString = TTauri::Text::Text_globals->unicode_data->toNFC(rhs, true, true);

    auto r = TTauri::Text::gstring{};
    auto breakState = TTauri::Text::GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (let codePoint : normalizedString) {
        if (TTauri::Text::Text_globals->unicode_data->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                r += TTauri::Text::Grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    if (ssize(cluster) != 0) {
        r += TTauri::Text::Grapheme{cluster};
    }
    return r;
}


}