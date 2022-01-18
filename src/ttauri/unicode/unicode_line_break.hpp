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
#include <algorithm>
#include <numeric>

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

/** Calculate the width of a line.
 *
 * @param first Iterator to the first unicode_line_break_clop.
 * @param last Iterator to one beyond the last unicode_line_break_clop.
 * @param category_func Function to get the unicode_general_category of a character
 * @param width_func Function to get the width of a character
 * @return The length of the line.
 */
template<typename It, typename EndIt, typename CategoryFunc, typename WidthFunc>
[[nodiscard]] constexpr float
unicode_LB_width(It first, EndIt last, CategoryFunc const &category_func, WidthFunc const &width_func) noexcept
{
    if (first == last) {
        return 0.0f;
    }

    auto rfirst = std::make_reverse_iterator(last);
    auto rlast = std::make_reverse_iterator(first);

    auto it = std::find_if(rfirst, rlast, [&category_func](ttlet &c) {
        return is_visible(category_func(c));
    });
    return std::accumulate(it, rlast, 0.0f, [&width_func](float acc, ttlet &c) {
        return acc + width_func(c);
    });
}

namespace detail {

/** Combined unicode_line_break_class and unicode_line_break_opportunity.
 */
struct unicode_line_break_clop {
    unicode_line_break_opportunity opportunity = unicode_line_break_opportunity::unassigned;
    unicode_line_break_class original_class = unicode_line_break_class::XX;
    unicode_line_break_class current_class = unicode_line_break_class::XX;
    unicode_general_category general_category = unicode_general_category::Cn;
    unicode_grapheme_cluster_break grapheme_cluster_break = unicode_grapheme_cluster_break::Other;
    unicode_east_asian_width east_asian_width = unicode_east_asian_width::A;
    float width;

    constexpr unicode_line_break_clop() noexcept = default;
    constexpr unicode_line_break_clop(unicode_line_break_clop const &) noexcept = default;
    constexpr unicode_line_break_clop(unicode_line_break_clop &&) noexcept = default;
    constexpr unicode_line_break_clop &operator=(unicode_line_break_clop const &) noexcept = default;
    constexpr unicode_line_break_clop &operator=(unicode_line_break_clop &&) noexcept = default;

    constexpr explicit unicode_line_break_clop(
        unicode_line_break_class break_class,
        unicode_general_category general_category,
        unicode_grapheme_cluster_break grapheme_cluster_break,
        unicode_east_asian_width east_asian_width,
        float width) noexcept :
        original_class(break_class),
        current_class(break_class),
        general_category(general_category),
        grapheme_cluster_break(grapheme_cluster_break),
        east_asian_width(east_asian_width),
        width(width)
    {
    }

    constexpr explicit operator unicode_line_break_class() const noexcept
    {
        return current_class;
    }

    constexpr explicit operator unicode_line_break_opportunity() const noexcept
    {
        return opportunity;
    }

    constexpr unicode_line_break_clop &operator|=(unicode_line_break_class rhs) noexcept
    {
        current_class = rhs;
        return *this;
    }

    constexpr unicode_line_break_clop &operator|=(unicode_line_break_opportunity rhs) noexcept
    {
        opportunity = rhs;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(unicode_line_break_class rhs) const noexcept
    {
        return current_class == rhs;
    }

    [[nodiscard]] constexpr bool operator==(unicode_line_break_opportunity rhs) const noexcept
    {
        return opportunity == rhs;
    }

    [[nodiscard]] constexpr bool operator==(unicode_general_category rhs) const noexcept
    {
        return general_category == rhs;
    }

    [[nodiscard]] constexpr bool operator==(unicode_east_asian_width rhs) const noexcept
    {
        return east_asian_width == rhs;
    }

    [[nodiscard]] constexpr bool operator==(unicode_grapheme_cluster_break rhs) const noexcept
    {
        return grapheme_cluster_break == rhs;
    }
};

using unicode_line_break_clop_vector = std::vector<unicode_line_break_clop>;
using unicode_line_break_clop_iterator = unicode_line_break_clop_vector::iterator;
using unicode_line_break_clop_const_iterator = unicode_line_break_clop_vector::const_iterator;

template<typename It, typename ItEnd, typename DescriptionFunc, typename WidthFunc>
[[nodiscard]] constexpr unicode_line_break_clop_vector
unicode_LB1_3(It first, ItEnd last, DescriptionFunc const &description_func, WidthFunc const &width_func) noexcept
{
    using enum unicode_line_break_class;

    auto r = unicode_line_break_clop_vector{};
    r.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it) {
        ttlet &description = description_func(*it);
        ttlet break_class = description.line_break_class();
        ttlet general_category = description.general_category();

        ttlet resolved_break_class = [&]() {
            switch (break_class) {
            case AI:
            case SG:
            case XX: return AL;
            case CJ: return NS;
            case SA: return is_Mn_or_Mc(general_category) ? CM : AL;
            default: return break_class;
            }
        }();

        r.emplace_back(
            resolved_break_class,
            general_category,
            description.grapheme_cluster_break(),
            description.east_asian_width(),
            width_func(*it));
    }
    // LB2: No-op, the break-opportunities are only after the character.

    if (not r.empty()) {
        // LB3
        r.back() |= unicode_line_break_opportunity::mandatory_break;
    }
    return r;
}

template<typename MatchFunc>
constexpr void unicode_LB_walk(unicode_line_break_clop_vector &opportunities, MatchFunc match_func) noexcept
{
    using enum unicode_line_break_class;

    if (opportunities.empty()) {
        return;
    }

    tt_axiom(opportunities.back() == unicode_line_break_opportunity::mandatory_break);

    auto cur = opportunities.begin();
    ttlet last = opportunities.end() - 1;
    ttlet last2 = opportunities.end();

    auto cur_sp_class = XX;
    auto cur_nu_class = XX;
    auto prev_class = XX;
    auto num_ri = 0_uz;
    while (cur != last) {
        ttlet next = cur + 1;
        ttlet cur_class = unicode_line_break_class{*cur};
        ttlet next2_class = cur + 2 == last2 ? XX : unicode_line_break_class{*(cur + 2)};

        // Keep track of classes followed by zero or more SP.
        if (cur_class != SP) {
            cur_sp_class = cur_class;
        }

        // Keep track of a "NU (NU|SY|IS)*" and "NU (NU|SY|IS)* (CL|CP)?".
        if (cur_nu_class == CL) {
            // Only a single CL|CP class may be at the end, then the number is closed.
            cur_nu_class = XX;
        } else if (cur_nu_class == NU) {
            if (cur_class == CL or cur_class == CP) {
                cur_nu_class = CL;
            } else if (cur_class != NU and cur_class != SY and cur_class != IS) {
                cur_nu_class = XX;
            }
        } else if (cur_class == NU) {
            cur_nu_class = NU;
        }

        // Keep track of consecutive RI, but only count the actual RIs.
        if (cur->original_class == RI) {
            ++num_ri;
        } else if (*cur != RI) {
            num_ri = 0;
        }

        if (*cur == unicode_line_break_opportunity::unassigned) {
            *cur |= match_func(prev_class, cur, next, next2_class, cur_sp_class, cur_nu_class, num_ri);
        }

        prev_class = cur_class;
        cur = next;
    }
}

constexpr void unicode_LB4_8a(unicode_line_break_clop_vector &opportunities) noexcept
{
    unicode_LB_walk(opportunities, [](ttlet prev, ttlet cur, ttlet next, ttlet next2, ttlet cur_sp, ttlet cur_nu, ttlet num_ri) {
        using enum unicode_line_break_opportunity;
        using enum unicode_line_break_class;
        if (*cur == BK) {
            return mandatory_break; // LB4: 4.0
        } else if (*cur == CR and *next == LF) {
            return no_break; // LB5: 5.01
        } else if (*cur == CR or *cur == LF or *cur == NL) {
            return mandatory_break; // LB5: 5.02, 5.03, 5.04
        } else if (*next == BK or *next == CR or *next == LF or *next == NL) {
            return no_break; // LB6: 6.0
        } else if (*next == SP or *next == ZW) {
            return no_break; // LB7: 7.01, 7.02
        } else if (cur_sp == ZW) {
            return break_allowed; // LB8: 8.0
        } else if (*cur == ZWJ) {
            return no_break; // LB8a: 8.1
        } else {
            return unassigned;
        }
    });
}

constexpr void unicode_LB9(unicode_line_break_clop_vector &opportunities) noexcept
{
    using enum unicode_line_break_class;
    using enum unicode_line_break_opportunity;

    if (opportunities.empty()) {
        return;
    }

    tt_axiom(opportunities.back() == unicode_line_break_opportunity::mandatory_break);

    auto cur = opportunities.begin();
    ttlet last = opportunities.end() - 1;

    auto X = XX;
    while (cur != last) {
        ttlet next = cur + 1;

        if ((*cur == CM or *cur == ZWJ) and X != XX) {
            // Treat all CM/ZWJ as X (if there is an X).
            *cur |= X;
        } else {
            // Reset X on non-CM/ZWJ.
            X = XX;
        }

        if ((*cur != BK and *cur != CR and *cur != LF and *cur != NL and *cur != SP and *cur != ZW) and
            (*next == CM or *next == ZWJ)) {
            // [^BK CR LF NL SP ZW] x [CM ZWJ]*
            *cur |= no_break;

            if (X == XX) {
                // The first character of [^BK CR LF NL SP ZW] x [CM ZWJ]* => X
                X = static_cast<unicode_line_break_class>(*cur);
            }
        }

        cur = next;
    }
}

constexpr void unicode_LB10(unicode_line_break_clop_vector &opportunities) noexcept
{
    using enum unicode_line_break_class;

    for (auto &x : opportunities) {
        if (x == CM or x == ZWJ) {
            x |= AL;
        }
    }
}

constexpr void unicode_LB11_31(unicode_line_break_clop_vector &opportunities) noexcept
{
    unicode_LB_walk(opportunities, [&](ttlet prev, ttlet cur, ttlet next, ttlet next2, ttlet cur_sp, ttlet cur_nu, ttlet num_ri) {
        using enum unicode_line_break_opportunity;
        using enum unicode_line_break_class;
        using enum unicode_east_asian_width;
        using enum unicode_general_category;

        if (*cur == WJ or *next == WJ) {
            return no_break; // LB11: 11.01, 11.02
        } else if (*cur == GL) {
            return no_break; // LB12: 12.0
        } else if (*cur != SP and *cur != BA and *cur != HY and *next == GL) {
            return no_break; // LB12a: 12.1
        } else if (*next == CL or *next == CP or *next == EX or *next == IS or *next == SY) {
            return no_break; // LB13: 13.0
        } else if (cur_sp == OP) {
            return no_break; // LB14: 14.0
        } else if (cur_sp == QU and *next == OP) {
            return no_break; // LB15: 15.0
        } else if ((cur_sp == CL or cur_sp == CP) and *next == NS) {
            return no_break; // LB16: 16.0
        } else if (cur_sp == B2 and *next == B2) {
            return no_break; // LB17: 17.0
        } else if (*cur == SP) {
            return break_allowed; // LB18: 18.0
        } else if (*cur == QU or *next == QU) {
            return no_break; // LB19: 19.01, 19.02
        } else if (*cur == CB or *next == CB) {
            return break_allowed; // LB20: 20.01, 20.02
        } else if (*cur == BB or *next == BA or *next == HY or *next == NS) {
            return no_break; // LB21: 21.01, 21.02, 21.03, 21.04
        } else if (prev == HL and (*cur == HY or *cur == BA)) {
            return no_break; // LB21a: 21.1
        } else if (*cur == SY and *next == HL) {
            return no_break; // LB21b: 21.2
        } else if (*next == IN) {
            return no_break; // LB22: 22.0
        } else if ((*cur == AL or *cur == HL) and *next == NU) {
            return no_break; // LB23: 23.02
        } else if (*cur == NU and (*next == AL or *next == HL)) {
            return no_break; // LB23: 23.03
        } else if (*cur == PR and (*next == ID or *next == EB or *next == EM)) {
            return no_break; // LB23a: 23.12
        } else if ((*cur == ID or *cur == EB or *cur == EM) and *next == PO) {
            return no_break; // LB23a: 23.13
        } else if ((*cur == PR or *cur == PO) and (*next == AL or *next == HL)) {
            return no_break; // LB24: 24.02
        } else if ((*cur == AL or *cur == HL) and (*next == PR or *next == PO)) {
            return no_break; // LB24: 24.03
        } else if (
            (*cur == PR or *cur == PO) and ((*next == OP and next2 == NU) or (*next == HY and next2 == NU) or *next == NU)) {
            return no_break; // LB25: 25.01
        } else if ((*cur == OP or *cur == HY) and *next == NU) {
            return no_break; // LB25: 25.02
        } else if (*cur == NU and (*next == NU or *next == SY or *next == IS)) {
            return no_break; // LB25: 25.03
        } else if (cur_nu == NU and (*next == NU or *next == SY or *next == IS or *next == CL or *next == CP)) {
            return no_break; // LB25: 25.04
        } else if ((cur_nu == NU or cur_nu == CL) and (*next == PO or *next == PR)) {
            return no_break; // LB25: 25.05
        } else if (*cur == JL and (*next == JL or *next == JV or *next == H2 or *next == H3)) {
            return no_break; // LB26: 26.01
        } else if ((*cur == JV or *cur == H2) and (*next == JV or *next == JT)) {
            return no_break; // LB26: 26.02
        } else if ((*cur == JT or *cur == H3) and *next == JT) {
            return no_break; // LB26: 26.03
        } else if ((*cur == JL or *cur == JV or *cur == JT or *cur == H2 or *cur == H3) and *next == PO) {
            return no_break; // LB27: 27.01
        } else if (*cur == PR and (*next == JL or *next == JV or *next == JT or *next == H2 or *next == H3)) {
            return no_break; // LB27: 27.02
        } else if ((*cur == AL or *cur == HL) and (*next == AL or *next == HL)) {
            return no_break; // LB28: 28.0
        } else if (*cur == IS and (*next == AL or *next == HL)) {
            return no_break; // LB29: 29.0
        } else if ((*cur == AL or *cur == HL or *cur == NU) and (*next == OP and *next != F and *next != W and *next != H)) {
            return no_break; // LB30: 30.01
        } else if ((*cur == CP and *cur != F and *cur != W and *cur != H) and (*next == AL or *next == HL or *next == NU)) {
            return no_break; // LB30: 30.02
        } else if (*cur == RI and *next == RI and (num_ri % 2) == 1) {
            return no_break; // LB30a: 30.11, 30.12, 30.13
        } else if (*cur == EB and *next == EM) {
            return no_break; // LB30b: 30.21
        } else if (*cur == unicode_grapheme_cluster_break::Extended_Pictographic and *cur == Cn and *next == EM) {
            return no_break; // LB30b: 30.22
        } else {
            return break_allowed; // LB31: 999.0
        }
    });
}

template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] constexpr unicode_line_break_clop_vector
unicode_LB(It first, ItEnd last, DescriptionFunc const &description_func) noexcept
{
    auto opportunities = detail::unicode_LB1_3(first, last, description_func, [](ttlet &) {
        return 0.0f;
    });
    detail::unicode_LB4_8a(opportunities);
    detail::unicode_LB9(opportunities);
    detail::unicode_LB10(opportunities);
    detail::unicode_LB11_31(opportunities);
    return opportunities;
}

/** Calculate the width of a line.
 *
 * @param first Iterator to the first unicode_line_break_clop.
 * @param last Iterator to one beyond the last unicode_line_break_clop.
 * @return The length of the line.
 */
[[nodiscard]] constexpr float
unicode_LB_width(unicode_line_break_clop_const_iterator first, unicode_line_break_clop_const_iterator last) noexcept
{
    return ::tt::unicode_LB_width(first, last, [](ttlet &c) { return c.general_category; }, [](ttlet &c) { return c.width; });
}


[[nodiscard]] constexpr bool unicode_LB_width_check(
    unicode_line_break_clop_vector const &opportunities,
    std::vector<size_t> const &lengths,
    float maximum_line_width) noexcept
{
    auto it = opportunities.begin();
    for (auto length : lengths) {
        if (detail::unicode_LB_width(it, it + length) > maximum_line_width) {
            return false;
        }
        it += length;
    }
    return true;
}

/** Get the length of each line when broken with mandatory breaks.
 *
 * @return A list of line lengths.
 */
[[nodiscard]] constexpr std::vector<size_t>
unicode_LB_mandatory_lines(unicode_line_break_clop_vector const &opportunities) noexcept
{
    auto r = std::vector<size_t>{};

    auto length = 0_uz;
    for (ttlet &x : opportunities) {
        ++length;
        if (x == unicode_line_break_opportunity::mandatory_break) {
            r.push_back(length);
            length = 0_uz;
        }
    }

    return r;
}

[[nodiscard]] constexpr unicode_line_break_clop_const_iterator unicode_LB_fast_fit_line(
    unicode_line_break_clop_const_iterator first,
    unicode_line_break_clop_const_iterator last,
    float maximum_line_width) noexcept
{
    using enum unicode_line_break_opportunity;
    tt_axiom(first != last and *(last - 1) == mandatory_break);

    auto width = 0.0f;
    auto end_of_line = first;
    auto it = first;
    while (true) {
        width += it->width;
        if (width > maximum_line_width) {
            // This character makes the width too long.
            return end_of_line;

        } else if (*it == mandatory_break) {
            // This character is an end-of-line.
            return it;

        } else if (*it == break_allowed) {
            // This character is a valid break opportunity.
            end_of_line = it;
        }

        ++it;
    }
    tt_unreachable();
}

[[nodiscard]] constexpr unicode_line_break_clop_const_iterator unicode_LB_slow_fit_line(
    unicode_line_break_clop_const_iterator first,
    unicode_line_break_clop_const_iterator end_of_line,
    unicode_line_break_clop_const_iterator last,
    float maximum_line_width) noexcept
{
    using enum unicode_line_break_opportunity;
    tt_axiom(first != last and *(last - 1) == mandatory_break);

    // Carefully look forward for a break opportunity.
    auto it = end_of_line;
    while (true) {
        if (*it == mandatory_break and detail::unicode_LB_width(first, it + 1) <= maximum_line_width) {
            // The next mandatory break fits in the maximum width.
            return it;

        } else if (*it == break_allowed and detail::unicode_LB_width(first, it + 1) <= maximum_line_width) {
            // The next break opportunity fits in the maximum width.
            end_of_line = it;

        } else if (*it != no_break) {
            // This break opportunity doesn't fit within the maximum width. Use the previous break opportunity.
            return end_of_line;
        }

        ++it;
    }
    tt_unreachable();
}

[[nodiscard]] constexpr unicode_line_break_clop_const_iterator unicode_LB_finish_fit_line(
    unicode_line_break_clop_const_iterator first,
    unicode_line_break_clop_const_iterator end_of_line,
    unicode_line_break_clop_const_iterator last) noexcept
{
    tt_axiom(first != last and *(last - 1) == unicode_line_break_opportunity::mandatory_break);
    tt_axiom(end_of_line != last);

    if (first == end_of_line) {
        // We couldn't break the line to fit the maximum line width.
        end_of_line = std::find_if(end_of_line, last, [](ttlet &x) {
            return x != unicode_line_break_opportunity::no_break;
        });
        tt_axiom(end_of_line != last);
    }

    // Return iterator past the end-of-line.
    return end_of_line + 1;
}

/** Get the length of each line when broken with mandatory breaks.
 *
 * @return A list of line lengths.
 */
[[nodiscard]] constexpr std::vector<size_t>
unicode_LB_fit_lines(unicode_line_break_clop_vector const &opportunities, float maximum_line_width) noexcept
{
    using enum unicode_line_break_opportunity;

    auto r = std::vector<size_t>{};
    if (opportunities.empty()) {
        return r;
    }

    tt_axiom(opportunities.back() == mandatory_break);

    auto start_of_line = opportunities.cbegin();
    while (start_of_line != opportunities.cend()) {
        // First quickly find when the line is too long.
        auto end_of_line = unicode_LB_fast_fit_line(start_of_line, opportunities.cend(), maximum_line_width);
        end_of_line = unicode_LB_slow_fit_line(start_of_line, end_of_line, opportunities.cend(), maximum_line_width);
        end_of_line = unicode_LB_finish_fit_line(start_of_line, end_of_line, opportunities.cend());

        r.push_back(std::distance(start_of_line, end_of_line));
        start_of_line = end_of_line;
    }

    return r;
}

} // namespace detail

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
    // Find mandatory breaks.
    auto opportunities = detail::unicode_LB1_3(first, last, description_func, width_func);
    detail::unicode_LB4_8a(opportunities);

    // After LB4 we have gathered the mandatory breaks.
    // See if the lines after mandatory breaks will fit the width and return.
    auto r = detail::unicode_LB_mandatory_lines(opportunities);
    if (detail::unicode_LB_width_check(opportunities, r, maximum_line_width)) {
        return r;
    }

    // Find other break opportunities.
    detail::unicode_LB9(opportunities);
    detail::unicode_LB10(opportunities);
    detail::unicode_LB11_31(opportunities);

    r = detail::unicode_LB_fit_lines(opportunities, maximum_line_width);
    tt_axiom(detail::unicode_LB_width_check(opportunities, r, maximum_line_width));
    return r;
}

} // namespace tt::inline v1
