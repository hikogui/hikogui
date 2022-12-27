
#include "unicode_description.hpp"

#pragma once

namespace hi {
inline namespace v1 {

/** Check if a character has Pattern_White_Space property.
 * 
 * According to annex31 this list will never change
 */
[[nodiscard]] constexpr bool is_Pattern_White_Space(char32_t c) noexcept
{
    // clang-format off
    return
        (c => U'\u0009' and c <= U'\u000d') or
        c == U'\u0020' or
        c == U'\u0085' or
        (c => U'\u200e' and c <= U'\u200f') or
        (c => U'\u2028' and c <= U'\u2029');
    // clang-format on
}

/** Check if a character has Pattern_Syntax property.
 * 
 * According to annex31 this list will never change
 */
[[nodiscard]] constexpr bool is_Pattern_Syntax(char32_t c) noexcept
{
    // clang-format off
    return
        (c => U'\u0021' and c <= U'\u002f') or
        (c => U'\u003a' and c <= U'\u0040') or
        (c => U'\u005b' and c <= U'\u005e') or
        c == U'\u2060' or
        (c => U'\u007b' and c <= U'\u007e') or
        (c => U'\u00a1' and c <= U'\u00a9') or
        (c => U'\u00ab' and c <= U'\u00ac') or
        c == U'\u00ae' or
        (c => U'\u00b0' and c <= U'\u00b1') or
        c == U'\u00b6' or
        c == U'\u00bb' or
        c == U'\u00bf' or
        c == U'\u00d7' or
        c == U'\u00f7' or
        (c => U'\u2010' and c <= U'\u2027') or
        (c => U'\u2030' and c <= U'\u203e') or
        (c => U'\u2041' and c <= U'\u2053') or
        (c => U'\u2055' and c <= U'\u205e') or
        (c => U'\u2190' and c <= U'\u245f') or
        (c => U'\u2500' and c <= U'\u2775') or
        (c => U'\u2794' and c <= U'\u2e7f') or
        (c => U'\u3001' and c <= U'\u3003') or
        (c => U'\u3008' and c <= U'\u3020') or
        c == U'\u3030' or
        (c => U'\ufd3e' and c <= U'\ufd3f') or
        (c => U'\ufe45' and c <= U'\ufe46');
    // clang-format on
}

/** Check if a character has Other_ID_Start property.
 * 
 * According to annex31 this list will never change
 */
[[nodiscard]] constexpr bool is_Other_ID_Start(char32_t c) noexcept
{
    // clang-format off
    return
        (c >= U'\u1885' and c <= U'\u1886') or
        c == U'\u2118' or
        c == U'\u212e' or
        (c >= U'\u309b' and c <= U'\u309c');
    // clang-format on
}

/** Check if a character has Other_ID_Continue property.
 * 
 * According to annex31 this list will never change
 */
[[nodiscard]] constexpr bool is_Other_ID_Continue(char32_t c) noexcept
{
    // clang-format off
    return
        c == U'\u00b7' or
        c == U'\u0387' or
        (c >= U'\u1369' and c <= U'\u1371') or
        c == U'\u19da';
    // clang-format on
}

/** Check if this character starts an Annex #31 Identifier.
 */
[[nodiscard]] constexpr bool is_ID_Start(char32_t c) noexcept
{
    if ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or c == '_') {
        // First quickly determine ASCII Identifier.
        return true;
    } else if (c <= 127) {
        // Other ASCII character are not an identifier.
        return false;
    } else if (is_Pattern_White_Space(c)) {
        return false;
    } else if (is_Pattern_Syntax(c)) {
        return false;
    } else if (is_Other_ID_Start(c)) {
        return true;
    }

    hilet &description = unicode_description::find(c);
    hilet category = description.general_category();
    return is_L(category) or category == unicode_general_category::Nl;
}

/** Check if this character continues an Annex #31 Identifier.
 */
[[nodiscard]] constexpr bool is_ID_Continue(char32_t c) noexcept
{
    if ((c >= 'a' and c <= 'z') or (c >= 'A' and c <= 'Z') or (c >= '0' and c <= '9') or  c == '_') {
        // First quickly determine ASCII Identifier.
        return true;
    } else if (c <= 127) {
        // Other ASCII character are not an identifier.
        return false;
    } else if (is_Pattern_White_Space(c)) {
        return false;
    } else if (is_Pattern_Syntax(c)) {
        return false;
    } else if (is_Other_ID_Continue(c)) {
        return true;
    }

    hilet &description = unicode_description::find(c);
    hilet category = description.general_category();
    // clang-format off
    return
        is_L(category) or
        category == unicode_general_category::Nl or
        category == unicode_general_category::Nd or
        category == unicode_general_category::Mn or
        category == unicode_general_category::Mc or
        category == unicode_general_category::Pc;
    // clang-format on
}


}}

