// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode_line_break
 */

#pragma once

namespace tt::inline v1 {

enum class unicode_sentence_break_property {
    Other,
    CR,
    LF,
    Extend,
    Sep,
    Format,
    Sp,
    Lower,
    Upper,
    OLetter,
    Numeric,
    ATerm,
    SContinue,
    STerm,
    Close
};

[[nodiscard]] constexpr bool is_ParaSep(unicode_sentence_break_property rhs) noexcept
{
    return rhs == unicode_sentence_break_property::Sep or rhs == unicode_sentence_break_property::CR or rhs == unicode_sentence_break_property::LF;
}

[[nodiscard]] constexpr bool is_SATerm(unicode_sentence_break_property rhs) noexcept
{
    return rhs == unicode_sentence_break_property::STerm or rhs == unicode_sentence_break_property::ATerm;
}

enum class unicode_sentence_break_opportunity {
    unassigned,
    no_break,
    break_allowed
};

}
