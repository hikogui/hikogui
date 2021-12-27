// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode_line_break
 */

#pragma once

#include "unicode_general_category.hpp"
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

[[nodiscard]] constexpr bool any(unicode_line_break_opportunity rhs) noexcept
{
    return static_cast<bool>(to_underlying(rhs) & 0xc0);
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

template<typename It, typename ItEnd, typename CharInfoFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_opportunity>
unicode_LB1(It first, ItEnd last, CharInfoFunc const &char_info_func) noexcept
{
    auto r = std::vector<unicode_line_break_opportunity>{};
    r.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it) {
        ttlet[break_class, category, width] = char_info_func(*it);

        switch (break_class) {
            using enum unicode_line_break_class;
        case CB: // XXX CB is an embedded object that needs to be queried how to line-break.
        case AI:
        case SG:
        case XX: r.push_back(to_unicode_line_break_opportunity(AL)); break;
        case CJ: r.push_back(to_unicode_line_break_opportunity(NS)); break;
        case SA: r.push_back(to_unicode_line_break_opportunity(is_Mn_or_Mc(category) ? CM : AL)); break;
        default: r.push_back(to_unicode_line_break_opportunity(break_class));
        }
    }
    return r;
}

constexpr void unicode_LB2_3(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    // LB2: No-op, the opportunities are only after the character.

    tt_axiom(not opportunities.empty());
    tt_axiom(not any(opportunities.back()));
    // LB3
    opportunities.back() |= unicode_line_break_opportunity::mandatory_break;
}

constexpr void unicode_LB4_7(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            if (*it == BK) {
                // LB4
                *it |= unicode_line_break_opportunity::mandatory_break;
            } else if (*it == CR and *next == LF) {
                // LB5
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == CR || *it == LF || *it == NL) {
                // LB5
                *it |= unicode_line_break_opportunity::mandatory_break;
            } else if (*next == BK or *next == CR or *next == LF or *next == NL) {
                // LB6
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*next == SP or *next == ZW) {
                // LB7
                *it |= unicode_line_break_opportunity::no_break;
            }
        }

        it = next;
    }
}

constexpr void unicode_LB8(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    auto prev = it++;
    ttlet last = opportunities.end();

    auto found_zw = false;
    while (it != last) {
        if (found_zw) {
            if (*it != SP and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::break_allowed;
            }
            found_zw = *it == ZW or *it == SP;

        } else {
            found_zw = *it == ZW;
        }

        prev = it++;
    }
}

constexpr void unicode_LB8a(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    for (auto &x : opportunities) {
        if (not any(x) and x == unicode_line_break_class::ZWJ) {
            x |= unicode_line_break_opportunity::no_break;
        }
    }
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

constexpr void unicode_LB11_12(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;

    unicode_line_break_opportunity *prev = nullptr;
    for (auto &x : opportunities) {
        if (x == WJ) {
            // LB11
            if (not any(x)) {
                x |= unicode_line_break_opportunity::no_break;
            }
            if (prev and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::no_break;
            }
        } else if (x == GL) {
            // LB12
            if (not any(x)) {
                x |= unicode_line_break_opportunity::no_break;
            }
        }
    }
}

constexpr void unicode_LB12a_13(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            if (*it != SP and *it != BA and *it != HY and *next == GL) {
                // LB12a
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == CL or *it == CP or *it == EX or *it == IS or *it == SY) {
                // LB13
                *it |= unicode_line_break_opportunity::no_break;
            }
        }

        it = next;
    }
}

constexpr void unicode_LB14(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    auto prev = it++;
    ttlet last = opportunities.end();

    auto found_op = false;
    while (it != last) {
        if (found_op) {
            if (*it != SP and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::no_break;
            }
            found_op = *it == OP or *it == SP;

        } else {
            found_op = *it == OP;
        }

        prev = it++;
    }
}

constexpr void unicode_LB15(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    auto prev = it++;
    ttlet last = opportunities.end();

    auto found_qu = false;
    while (it != last) {
        if (found_qu) {
            if (*it == OP and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::no_break;
            }
            found_qu = *it == QU or *it == SP;

        } else {
            found_qu = *it == QU;
        }

        prev = it++;
    }
}

constexpr void unicode_LB16(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    auto prev = it++;
    ttlet last = opportunities.end();

    auto found_cl_cp = false;
    while (it != last) {
        if (found_cl_cp) {
            if (*it == NS and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::no_break;
            }
            found_cl_cp = *it == CL or *it == CP or *it == SP;

        } else {
            found_cl_cp = *it == CL or *it == CP;
        }

        prev = it++;
    }
}

constexpr void unicode_LB17(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    auto prev = it++;
    ttlet last = opportunities.end();

    auto found_b2 = false;
    while (it != last) {
        if (found_b2) {
            if (*it == B2 and not any(*prev)) {
                *prev |= unicode_line_break_opportunity::no_break;
            }
            found_b2 = *it == B2 or *it == SP;

        } else {
            found_b2 = *it == B2;
        }

        prev = it++;
    }
}

constexpr void unicode_LB18_21(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            if (*it == SP) {
                // LB18
                *it |= unicode_line_break_opportunity::break_allowed;
            } else if (*it == QU or *next == QU) {
                // LB19
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == CB or *next == CB) {
                // LB20
                *it |= unicode_line_break_opportunity::break_allowed;
            } else if (*it == BB or *next == BA or *next == HY or *next == NS) {
                // LB21
                *it |= unicode_line_break_opportunity::no_break;
            }
        }

        it = next;
    }
}

constexpr void unicode_LB21a(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    while (it != last) {
        ttlet next = it + 1;

        if (not any(*next)) {
            if (*it == HL and (*next == HY or *next == BA)) {
                *next |= unicode_line_break_opportunity::no_break;
            }
        }

        it = next;
    }
}

constexpr void unicode_LB21b_30(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            if (*it == SY and *next == HL) {
                // LB21b
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*next == IN) {
                // LB22
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == AL or *it == HL) and *next == NU) {
                // LB23.01
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == NU and (*next == AL or *next == HL)) {
                // LB23.02
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == PR and (*next == ID or *next == EB or *next == EM)) {
                // LB23a.01
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == ID or *it == EB or *it == EM) and *next == PO) {
                // LB23a.02
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == PR or *it == PO) and (*next == AL or *next == HL)) {
                // LB24.01
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == AL or *it == HL) and (*next == PR or *next == PO)) {
                // LB24.02
                *it |= unicode_line_break_opportunity::no_break;
            } else if (
                (*it == CL and *next == PO) or (*it == CP and *next == PO) or (*it == CL and *next == PR) or
                (*it == CP and *next == PR) or (*it == NU and *next == PO) or (*it == NU and *next == PR) or
                (*it == PO and *next == OP) or (*it == PO and *next == NU) or (*it == PR and *next == OP) or
                (*it == PR and *next == NU) or (*it == HY and *next == NU) or (*it == IS and *next == NU) or
                (*it == NU and *next == NU) or (*it == SY and *next == NU)) {
                // LB25
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == JL and (*next == JL or *next == JV or *next == H2 or *next == H3)) {
                // LB26.01
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == JV or *it == H2) and (*next == JV or *next == JT)) {
                // LB26.02
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == JT or *it == H3) and *next == JT) {
                // LB26.03
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == JL or *it == JV or *it == JT or *it == H2 or *it == H3) and *next == PO) {
                // LB27.01
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == PR and (*next == JL or *next == JV or *next == JT or *next == H2 or *next == H3)) {
                // LB27.02
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == AL or *it == HL) and (*next == AL or *next == HL)) {
                // LB28
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == IS and (*next == AL or *next == HL)) {
                // LB29
                *it |= unicode_line_break_opportunity::no_break;
            } else if ((*it == AL or *it == HL or *it == NU) and *next == OP) {
                // LB30.01
                // XXX incomplete
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == CP and (*next == AL or *next == HL or *next == NU)) {
                // LB30.02
                // XXX incomplete
                *it |= unicode_line_break_opportunity::no_break;
            }
        }

        it = next;
    }
}

constexpr void unicode_LB30a_31(std::vector<unicode_line_break_opportunity> &opportunities) noexcept
{
    using enum unicode_line_break_class;
    tt_axiom(not opportunities.empty());
    auto it = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    auto consequtive_ri = 0_uz;
    while (it != last) {
        ttlet next = it + 1;

        if (not any(*it)) {
            if (*it == RI and *next == RI and (consequtive_ri % 2) == 0) {
                // LB30a
                *it |= unicode_line_break_opportunity::no_break;
            } else if (*it == EB and *next == EM) {
                // LB30b XXX incomplete
                *it |= unicode_line_break_opportunity::no_break;
            } else {
                // LB31
                *it |= unicode_line_break_opportunity::break_allowed;
            }
        }
        
        if (*it == RI) {
            ++consequtive_ri;
        } else {
            consequtive_ri = 0;
        }

        it = next;
    }
}

} // namespace detail

template<typename It, typename ItEnd, typename UnicodeDescriptionFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_opportunity>
unicode_line_break_algorithm(It first, ItEnd last, UnicodeDescriptionFunc const &unicode_description_func) noexcept
{
    auto opportunities = detail::unicode_LB1(first, last, unicode_description_func);
    if (not opportunities.empty()) {
        detail::unicode_LB2_3(opportunities);
        detail::unicode_LB4_7(opportunities);
        detail::unicode_LB8(opportunities);
        detail::unicode_LB8a(opportunities);
        detail::unicode_LB9_10(opportunities);
        detail::unicode_LB11_12(opportunities);
        detail::unicode_LB12a_13(opportunities);
        detail::unicode_LB14(opportunities);
        detail::unicode_LB15(opportunities);
        detail::unicode_LB16(opportunities);
        detail::unicode_LB17(opportunities);
        detail::unicode_LB18_21(opportunities);
        detail::unicode_LB21a(opportunities);
        detail::unicode_LB21b_30(opportunities);
        detail::unicode_LB30a_31(opportunities);
    }
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
template<typename It, typename ItEnd, typename CharInfoFunc>
std::vector<size_t> unicode_break_lines(It first, ItEnd last, float maximum_line_width, CharInfoFunc const &char_info_func)
{
}

} // namespace tt::inline v1
