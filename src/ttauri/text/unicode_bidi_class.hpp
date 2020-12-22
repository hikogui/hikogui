// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstdint>

namespace tt {

/** Bidirectional class
 * Unicode Standard Annex #9: https://unicode.org/reports/tr9/
 */
enum class unicode_bidi_class : uint8_t {
    unknown = 0,
    L = 1, ///< Left-to-Right
    R = 2, ///< Right-to-Left
    AL = 3, ///< Right-to-Left Arabic
    EN = 4, ///< European Number
    ES = 5, ///< European Number Separator
    ET = 6, ///< European Number Terminator
    AN = 7, ///< Arabic Number
    CS = 8, ///< Common Number Separator
    NSM = 9, ///< Nonspacing Mark
    BN = 10, ///< Boundary Neutral
    B = 11, ///< Paragraph Separator
    S = 12, ///< Segment Separator
    WS = 13, ///< Whitespace
    ON = 14, ///< Other Neutrals
    // Explicit values.
    LRE, ///< Left-to-Right Embedding
    LRO, ///< Left-to-Right Override
    RLE, ///< Right-to-Left Embedding
    RLO, ///< Right-to-left Override
    PDF, ///< Pop Directional Format
    LRI, ///< Left-to-Right Isolate
    RLI, ///< Right-to-Left Isolate
    FSI, ///< First Strong Isolate
    PDI ///< Pop Directional Isolate
};

constexpr bool is_isolate_starter(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return rhs == LRI || rhs == RLI || rhs == FSI;
}

constexpr bool is_isolate_formatter(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return is_isolate_starter(rhs) || rhs == PDI;
}

constexpr bool is_NI(unicode_bidi_class const &rhs) noexcept
{
    using enum unicode_bidi_class;
    return rhs == B || rhs == S || rhs == WS || rhs == ON || rhs == FSI || rhs == LRI || rhs == RLI || rhs == PDI;
}

}
