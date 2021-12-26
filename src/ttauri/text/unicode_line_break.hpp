// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode_line_break
 */

#pragma once

#include <cstdint>

namespace tt::inline v1 {

// Windows.h adds a "IN" macro that is used in this enum.
#ifdef IN
#undef IN
#endif

/** Unicode line break class.
 *
 * See "AUX14: Unicode line break algorithm"
 * http://unicode.org/reports/tr14/
 *
 */
enum class unicode_line_break_class : uint8_t {
    BK, // Mandatory Break NL, PARAGRAPH SEPARATOR Cause a line break (after)
    CR, // Carriage Return CR Cause a line break (after), except between CR and LF
    LF, // Line Feed LF Cause a line break (after)
    CM, // Combining Mark Combining marks, control codes Prohibit a line break between the character and the preceding character
    NL, // Next Line NEL Cause a line break (after)
    SG, // Surrogate Surrogates Do not occur in well-formed text
    WJ, // Word Joiner WJ Prohibit line breaks before and after
    ZW, // Zero Width Space ZWSP Provide a break opportunity
    GL, // Non-breaking (Glue) CGJ, NBSP, ZWNBSP Prohibit line breaks before and after
    SP, // Space SPACE Enable indirect line breaks
    ZWJ, // Zero Width Joiner Zero Width Joiner Prohibit line breaks within joiner sequences Break Opportunities

    B2, // Break Opportunity Before and After Em dash Provide a line break opportunity before and after the character
    BA, // Break After Spaces, hyphens Generally provide a line break opportunity after the character
    BB, // Break Before Punctuation used in dictionaries Generally provide a line break opportunity before the character
    HY, // Hyphen HYPHEN-MINUS Provide a line break opportunity after the character, except in numeric context
    CB, // Contingent Break Opportunity Inline objects Provide a line break opportunity contingent on additional information
        // Characters Prohibiting Certain Breaks

    CL, // Close Punctuation Prohibit line breaks before
    CP, // Close Parenthesis ')', ']' Prohibit line breaks before
    EX, // Exclamation/Interrogation '!', '?', etc. Prohibit line breaks before
    IN, // Inseparable Leaders Allow only indirect line breaks between pairs
    NS, // Nonstarter. Allow only indirect line breaks before
    OP, // Open Punctuation '(', '[', '{', etc. Prohibit line breaks after
    QU, // Quotation Quotation marks Act like they are both opening and closing Numeric Context

    IS, // Infix Numeric Separator . , Prevent breaks after any and before numeric
    NU, // Numeric Digits Form numeric expressions for line breaking purposes
    PO, // Postfix Numeric. Do not break following a numeric expression
    PR, // Prefix Numeric. Do not break in front of a numeric expression
    SY, // Symbols Allowing Break After / Prevent a break before, and allow a break after Other Characters

    AI, // Ambiguous (Alphabetic or Ideographic) Characters with Ambiguous East Asian Width Act like AL when the resolved EAW
        // is N; otherwise, act as ID
    AL, // Alphabetic Alphabets and regular symbols Are alphabetic characters or symbols that are used with alphabetic
        // characters
    CJ, // Conditional Japanese Starter Small kana Treat as NS or ID for strict or normal breaking.
    EB, // Emoji Base All emoji allowing modifiers Do not break from following Emoji Modifier
    EM, // Emoji Modifier Skin tone modifiers Do not break from preceding Emoji Base
    H2, // Hangul LV Syllable Hangul Form Korean syllable blocks
    H3, // Hangul LVT Syllable Hangul Form Korean syllable blocks
    HL, // Hebrew Letter Hebrew Do not break around a following hyphen; otherwise act as Alphabetic
    ID, // Ideographic Ideographs Break before or after, except in some numeric context
    JL, // Hangul
    L, // Jamo Conjoining jamo Form Korean syllable blocks
    JV, // Hangul
    V, // Jamo Conjoining jamo Form Korean syllable blocks
    JT, // Hangul
    T, // Jamo Conjoining jamo Form Korean syllable blocks
    RI, // Regional Indicator REGIONAL INDICATOR SYMBOL LETTER A..Z Keep pairs together.For pairs, break before and after other
        // classes
    SA, // Complex Context Dependent(South East Asian) South East Asian :Thai,Lao,Khmer Provide a line break opportunity
        // contingent on additional, language - specific context analysis
    XX, // Unknown Most unassigned, private - use Have as yet unknown line breaking behavior or unassigned code positions
};

} // namespace tt::inline v1
