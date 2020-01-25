// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/gstring.hpp"
#include "TTauri/Text/globals.hpp"
#include "TTauri/Foundation/strings.hpp"

namespace TTauri {

template<>
TTauri::Text::gstring translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    let normalizedString = TTauri::Text::Text_globals->unicode_data->toNFC(inputString, true, true);

    auto outputString = TTauri::Text::gstring{};
    auto breakState = TTauri::Text::GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (let codePoint : normalizedString) {
        if (TTauri::Text::Text_globals->unicode_data->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                outputString += TTauri::Text::Grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    outputString += TTauri::Text::Grapheme{cluster};
    return outputString;
}

template<>
std::u32string translateString(const TTauri::Text::gstring& inputString, TranslateStringOptions options) noexcept
{
    std::u32string outputString;

    for (let c : inputString) {
        outputString += c.NFC();
    }
    return outputString;
}

}