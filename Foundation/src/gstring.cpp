// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/gstring.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri {

template<>
gstring translateString(std::u32string_view const inputString, TranslateStringOptions options) noexcept
{
    let normalizedString = Foundation_globals->unicodeData->toNFC(inputString, true, true);

    auto outputString = gstring{};
    auto breakState = GraphemeBreakState{};
    auto cluster = std::u32string{};

    for (let codePoint : normalizedString) {
        if (Foundation_globals->unicodeData->checkGraphemeBreak(codePoint, breakState)) {
            if (cluster.size() > 0) {
                outputString += Grapheme{cluster};
            }
            cluster.clear();
        }

        cluster += codePoint;
    }
    outputString += Grapheme{cluster};
    return outputString;
}

template<>
std::u32string translateString(const gstring& inputString, TranslateStringOptions options) noexcept
{
    std::u32string outputString;

    for (let c : inputString) {
        outputString += c.NFC();
    }
    return outputString;
}

}