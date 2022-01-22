// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode_line_break
 */

#pragma once

namespace tt::inline v1 {

enum class unicode_word_break_property {
    Other,
    CR,
    LF,
    Newline,
    Extend,
    ZWJ,
    Regional_Indicator,
    Format,
    Katakana,
    Hebrew_Letter,
    ALetter,
    Single_Quote,
    Double_Quote,
    MidNumLet,
    MidLetter,
    MidNum,
    Numeric,
    ExtendNumLet,
    WSegSpace
};

[[nodiscard]] constexpr bool is_AHLetter(unicode_word_break_property rhs) noexcept
{
    return rhs == unicode_word_break_property::ALetter or rhs == unicode_word_break_property::Hebrew_Letter;
}

[[nodiscard]] constexpr bool is_MidNumLetQ(unicode_word_break_property rhs) noexcept
{
    return rhs == unicode_word_break_property::MidNumLet or rhs == unicode_word_break_property::Single_Quote;
}

enum class unicode_word_break_opportunity {
    unassigned,
    no_break,
    break_allowed
};

}
