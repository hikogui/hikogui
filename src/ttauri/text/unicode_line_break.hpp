// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode_line_break
 */

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_east_asian_width.hpp"
#include "../required.hpp"
#include "../cast.hpp"
#include "../assert.hpp"
#include <cstdint>
#include <vector>

// Windows.h adds a "IN" macro that is used in this enum.
#ifdef IN
#undef IN
#endif

namespace tt::inline v1 {

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

/** The opportunity for a line-break.
 *
 * This enum only uses the top 2 bits, and can be combined with the 6 bit `unicode_line_break_class`
 * into a single byte. This helps with performance in several ways.
 *  - Only single allocation is needed for both the temporary and return value of the opportunity list.
 *  - We can use a single iterator in the loop to walk both the break-opportunity and line-break-class.
 *  - Half the memory usage will reduce cache usage.
 */
enum class unicode_line_break_opportunity : uint8_t {
    unassigned = 0x00,
    mandatory_break = 0x40,
    no_break = 0x80,
    break_allowed = 0xc0
};

/** Strip away the lower 6 bits, leaving on valid values.
 */
[[nodiscard]] constexpr unicode_line_break_opportunity clean(unicode_line_break_opportunity rhs) noexcept
{
    return static_cast<unicode_line_break_opportunity>(to_underlying(rhs) & 0xc0);
}

[[nodiscard]] constexpr bool any(unicode_line_break_opportunity rhs) noexcept
{
    return static_cast<bool>(to_underlying(clean(rhs)));
}

[[nodiscard]] constexpr unicode_line_break_opportunity to_unicode_line_break_opportunity(unicode_line_break_class rhs) noexcept
{
    return static_cast<unicode_line_break_opportunity>(to_underlying(rhs));
}

[[nodiscard]] constexpr unicode_line_break_opportunity
operator|(unicode_line_break_opportunity const &lhs, unicode_line_break_opportunity const &rhs) noexcept
{
    return static_cast<unicode_line_break_opportunity>(to_underlying(lhs) | to_underlying(rhs));
}

constexpr unicode_line_break_opportunity &
operator|=(unicode_line_break_opportunity &lhs, unicode_line_break_opportunity const &rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

namespace detail {

[[nodiscard]] constexpr bool operator==(unicode_line_break_opportunity const &lhs, unicode_line_break_class const &rhs) noexcept
{
    tt_axiom(to_underlying(lhs) <= 0x3f);
    tt_axiom(to_underlying(rhs) <= 0x3f);
    return to_underlying(lhs) == to_underlying(rhs);
}

[[nodiscard]] constexpr unicode_line_break_class to_unicode_line_break_class(unicode_line_break_opportunity const &rhs) noexcept
{
    return static_cast<unicode_line_break_class>(to_underlying(rhs) & 0x3f);
}

constexpr unicode_line_break_class exchange(unicode_line_break_opportunity &lhs, unicode_line_break_class const &rhs) noexcept
{
    ttlet tmp = to_unicode_line_break_class(lhs);
    lhs = static_cast<unicode_line_break_opportunity>((to_underlying(lhs) & 0xc0) | to_underlying(rhs));
    return tmp;
}

template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_opportunity>
unicode_LB1_3(It first, ItEnd last, DescriptionFunc const &description_func) noexcept
{
    auto r = std::vector<unicode_line_break_opportunity>{};
    r.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it) {
        ttlet &description = description_func(*it);
        ttlet break_class = description.break_class();

        switch (break_class) {
            using enum unicode_line_break_class;
        case CB: // XXX CB is an embedded object that needs to be queried how to line-break.
        case AI:
        case SG:
        case XX: r.push_back(to_unicode_line_break_opportunity(AL)); break;
        case CJ: r.push_back(to_unicode_line_break_opportunity(NS)); break;
        case SA: r.push_back(to_unicode_line_break_opportunity(is_Mn_or_Mc(description.general_category()) ? CM : AL)); break;
        default: r.push_back(to_unicode_line_break_opportunity(break_class));
        }
    }
    // LB2: No-op, the break-opportunities are only after the character.

    if (not r.empty()) {
        // LB3
        r.back() |= unicode_line_break_opportunity::mandatory_break;
    }
    return r;
}

template<typename MatchFunc>
constexpr void unicode_LB_walk(std::vector<unicode_line_break_opportunity> &opportunities, MatchFunc match_func) noexcept
{
    using enum unicode_line_break_class;

    if (opportunities.empty()) {
        return;
    }

    tt_axiom(clean(opportunities.back()) == unicode_line_break_opportunity::mandatory_break);

    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    auto cur_sp_class = XX;
    auto prev_class = XX;
    while (it != last) {
        ttlet next = it + 1;

        // Keep track of classes followed by zero or more spaces.
        ttlet cur_class = to_unicode_line_break_class(*it);
        if (cur_class != SP) {
            cur_sp_class = cur_class;
        }

        if (not any(*it)) {
            ttlet next_class = to_unicode_line_break_class(*next);
            *it |= match_func(prev_class, cur_class, next_class, cur_sp_class);
        }

        prev_class = cur_class;
        it = next;
    }
}

constexpr void unicode_LB4_8a(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    unicode_LB_walk(opportunities, [](ttlet prev, ttlet cur, ttlet next, ttlet cur_sp) {
        using enum unicode_line_break_opportunity;
        using enum unicode_line_break_class;
        if (cur == BK) {
            // LB4
            return mandatory_break;
        } else if (cur == CR and next == LF) {
            // LB5
            return no_break;
        } else if (cur == CR or cur == LF or cur == NL) {
            // LB5
            return mandatory_break;
        } else if (next == BK or next == CR or next == LF or next == NL) {
            // LB6
            return no_break;
        } else if (next == SP or next == ZW) {
            // LB7
            return no_break;
        } else if (cur_sp == ZW) {
            // LB8
            return break_allowed;
        } else if (cur == ZWJ) {
            // LB8a
            return no_break;
        } else {
            return unassigned;
        }
    });
}

constexpr void unicode_LB9_10(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;

    auto found = unicode_line_break_class::XX;
    for (auto &x : opportunities) {
        // LB9
        if (found == XX) {
            found = (x != BK and x != CR and x != LF and x != NL and x != SP and x != ZW) ? to_unicode_line_break_class(x) : XX;

        } else {
            if (x == CM or x == ZWJ) {
                exchange(x, found);
            } else {
                found = unicode_line_break_class::XX;
            }
        }

        // LB10
        if (x == CM or x == ZWJ) {
            exchange(x, AL);
        }
    }
}

constexpr void unicode_LB11_29(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    unicode_LB_walk(opportunities, [](ttlet prev, ttlet cur, ttlet next, ttlet cur_sp) {
        using enum unicode_line_break_opportunity;
        using enum unicode_line_break_class;
        if (cur == WJ or next == WJ) {
            // LB11
            return no_break;
        } else if (cur == GL) {
            // LB12
            return no_break;
        } else if (cur != SP and cur != BA and cur != HY and next == GL) {
            // LB12a
            return no_break;
        } else if (cur == CL or cur == CP or cur == EX or cur == IS or cur == SY) {
            // LB13
            return no_break;
        } else if (cur_sp == OP) {
            // LB14
            return no_break;
        } else if (cur_sp == QU and next == OP) {
            // LB15
            return no_break;
        } else if ((cur_sp == CL or cur_sp == CP) and next == NS) {
            // LB16
            return no_break;
        } else if (cur_sp == B2 and next == B2) {
            // LB17
            return no_break;
        } else if (cur == SP) {
            // LB18
            return break_allowed;
        } else if (cur == QU or next == QU) {
            // LB19
            return no_break;
        } else if (cur == CB or next == CB) {
            // LB20
            return break_allowed;
        } else if (cur == BB or next == BA or next == HY or next == NS) {
            // LB21
            return no_break;
        } else if (prev == HL and (cur == HY or cur == BA)) {
            // LB21a
            return no_break;
        } else if (cur == SY and next == HL) {
            // LB21b
            return no_break;
        } else if (next == IN) {
            // LB22
            return no_break;
        } else if ((cur == AL or cur == HL) and next == NU) {
            // LB23.01
            return no_break;
        } else if (cur == NU and (next == AL or next == HL)) {
            // LB23.02
            return no_break;
        } else if (cur == PR and (next == ID or next == EB or next == EM)) {
            // LB23a.01
            return no_break;
        } else if ((cur == ID or cur == EB or cur == EM) and next == PO) {
            // LB23a.02
            return no_break;
        } else if ((cur == PR or cur == PO) and (next == AL or next == HL)) {
            // LB24.01
            return no_break;
        } else if ((cur == AL or cur == HL) and (next == PR or next == PO)) {
            // LB24.02
            return no_break;
        } else if (
            (cur == CL and next == PO) or (cur == CP and next == PO) or (cur == CL and next == PR) or
            (cur == CP and next == PR) or (cur == NU and next == PO) or (cur == NU and next == PR) or
            (cur == PO and next == OP) or (cur == PO and next == NU) or (cur == PR and next == OP) or
            (cur == PR and next == NU) or (cur == HY and next == NU) or (cur == IS and next == NU) or
            (cur == NU and next == NU) or (cur == SY and next == NU)) {
            // LB25
            return no_break;
        } else if (cur == JL and (next == JL or next == JV or next == H2 or next == H3)) {
            // LB26.01
            return no_break;
        } else if ((cur == JV or cur == H2) and (next == JV or next == JT)) {
            // LB26.02
            return no_break;
        } else if ((cur == JT or cur == H3) and next == JT) {
            // LB26.03
            return no_break;
        } else if ((cur == JL or cur == JV or cur == JT or cur == H2 or cur == H3) and next == PO) {
            // LB27.01
            return no_break;
        } else if (cur == PR and (next == JL or next == JV or next == JT or next == H2 or next == H3)) {
            // LB27.02
            return no_break;
        } else if ((cur == AL or cur == HL) and (next == AL or next == HL)) {
            // LB28
            return no_break;
        } else if (cur == IS and (next == AL or next == HL)) {
            // LB29
            return no_break;
        } else {
            return unassigned;
        }
    });
}

template<typename It, typename DescriptionFunc>
constexpr void unicode_LB30_31(
    std::vector<unicode_line_break_opportunity> &opportunities,
    It first,
    DescriptionFunc const &description_func) noexcept
{
    if (opportunities.empty()) {
        return;
    }

    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    auto consequtive_ri = 0_uz;
    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            *it |= []() {
                using enum unicode_line_break_class;
                using enum unicode_line_break_opportunity;
                if ((*it == AL or *it == HL or *it == NU) and *next == OP) {
                    using enum unicode_east_asian_width;
                    ttlet char_next = first + std::distance(opportunities.begin(), next);
                    ttlet &description = description_func(*char_next);
                    ttlet ea = description.east_asian_width();
                    if (ea != F and ea != W and ea != H) {
                        // LB30.01
                        return no_break;
                    }
                }
                if (*it == CP and (*next == AL or *next == HL or *next == NU)) {
                    using enum unicode_east_asian_width;
                    ttlet char_it = first + std::distance(opportunities.begin(), it);
                    ttlet &description = description_func(*char_it);
                    ttlet ea = description.east_asian_width();
                    if (ea != F and ea != W and ea != H) {
                        // LB30.02
                        return no_break;
                    }
                }
                if (*it == RI and *next == RI and (consequtive_ri % 2) == 0) {
                    // LB30a
                    return no_break;
                }
                if (*it == EB and *next == EM) {
                    // LB30b.01
                    return no_break;
                }
                if (*next == EM) {
                    ttlet char_it = first + std::distance(opportunities.begin(), it);
                    ttlet &description = description_func(*char_it);
                    if (description.grapheme_cluster_break() == unicode_grapheme_cluster_break::Extended_Pictographic or
                        description.general_category() == unicode_general_category::Cn) {
                        // LB30b.02
                        return no_break;
                    }
                }

                // LB31
                return break_allowed;
            }();
        }

        if (*it == unicode_line_break_class::RI) {
            ++consequtive_ri;
        } else {
            consequtive_ri = 0;
        }

        it = next;
    }
}

} // namespace detail

template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_opportunity>
unicode_line_break_algorithm(It first, ItEnd last, DescriptionFunc const &description_function) noexcept
{
    auto opportunities = detail::unicode_LB1_3(first, last, description_function);
    detail::unicode_LB4_8a(opportunities);
    detail::unicode_LB9_10(opportunities);
    detail::unicode_LB11_29(opportunities);
    detail::unicode_LB30_31(opportunities, description_function);
    return opportunities;
}

/** Unicode break lines.
 *
 * @tparam CharInfoFunc function with signature `std::pair<float, unicode_description const &>(decltype(*It))`
 * @param first Iterator to the first character.
 * @param last Iterator to one beyond the last character.
 * @param maximum_line_width The maximum line width.
 * @param char_info_func Function converting item dereferenced from a iterator to a width, description
 * @return A list of line lengths.
 */
template<typename It, typename ItEnd, typename DescriptionFunc, typename WidthFunc>
std::vector<size_t> unicode_break_lines(
    It first,
    ItEnd last,
    float maximum_line_width,
    DescriptionFunc const &description_func,
    WidthFunc const &width_func)
{
}

} // namespace tt::inline v1
