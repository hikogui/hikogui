// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_line_break.hpp
 */

#pragma once

#include "unicode_general_category.hpp"
#include "unicode_grapheme_cluster_break.hpp"
#include "unicode_east_asian_width.hpp"
#include "unicode_break_opportunity.hpp"
#include "../utility.hpp"
#include "../cast.hpp"
#include "../assert.hpp"
#include "../math.hpp"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>

// Windows.h adds a "IN" macro that is used in this enum.
#ifdef IN
#undef IN
#endif

namespace hi::inline v1 {

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
    SY, // Symbols Allowing Break After / Prevent a break before, and allow a break after XX Characters

    AI, // Ambiguous (Alphabetic or Ideographic) Characters with Ambiguous East Asian Width Act like AL when the resolved EAW
        // is N; XXwise, act as ID
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
    RI, // Regional Indicator REGIONAL INDICATOR SYMBOL LETTER A..Z Keep pairs together.For pairs, break before and after XX
        // classes
    SA, // Complex Context Dependent(South East Asian) South East Asian :Thai,Lao,Khmer Provide a line break opportunity
        // contingent on additional, language - specific context analysis
    XX, // Unknown Most unassigned, private - use Have as yet unknown line breaking behavior or unassigned code positions
};

/** Calculate the width of a line.
 *
 * @param first Iterator to the first character widths.
 * @param last Iterator to one beyond the last character width.
 * @return The length of the line.
 */
[[nodiscard]] constexpr float
unicode_line_break_width(std::vector<float>::const_iterator first, std::vector<float>::const_iterator last) noexcept
{
    if (first == last) {
        return 0.0f;
    }

    auto rfirst = std::make_reverse_iterator(last);
    auto rlast = std::make_reverse_iterator(first);

    auto it = std::find_if(rfirst, rlast, [](hilet &width) {
        return width >= 0.0;
    });
    return std::accumulate(it, rlast, 0.0f, [](float acc, hilet &width) {
        return acc + abs(width);
    });
}

namespace detail {

/** Combined unicode_line_break_class and unicode_line_break_opportunity.
 */
struct unicode_line_break_info {
    unicode_line_break_class original_class = unicode_line_break_class::XX;
    unicode_line_break_class current_class = unicode_line_break_class::XX;
    bool is_extended_pictographic = false;
    bool is_Cn = false;
    unicode_east_asian_width east_asian_width = unicode_east_asian_width::A;

    constexpr unicode_line_break_info() noexcept = default;
    constexpr unicode_line_break_info(unicode_line_break_info const &) noexcept = default;
    constexpr unicode_line_break_info(unicode_line_break_info &&) noexcept = default;
    constexpr unicode_line_break_info &operator=(unicode_line_break_info const &) noexcept = default;
    constexpr unicode_line_break_info &operator=(unicode_line_break_info &&) noexcept = default;

    constexpr explicit unicode_line_break_info(
        unicode_line_break_class break_class,
        bool is_Cn,
        bool is_extended_pictographic,
        unicode_east_asian_width east_asian_width) noexcept :
        original_class(break_class),
        current_class(break_class),
        is_Cn(is_Cn),
        is_extended_pictographic(is_extended_pictographic),
        east_asian_width(east_asian_width)
    {
    }

    constexpr explicit operator unicode_line_break_class() const noexcept
    {
        return current_class;
    }

    constexpr unicode_line_break_info &operator|=(unicode_line_break_class rhs) noexcept
    {
        current_class = rhs;
        return *this;
    }

    [[nodiscard]] constexpr bool operator==(unicode_line_break_class rhs) const noexcept
    {
        return current_class == rhs;
    }

    [[nodiscard]] constexpr bool operator==(unicode_east_asian_width rhs) const noexcept
    {
        return east_asian_width == rhs;
    }
};

using unicode_line_break_info_vector = std::vector<unicode_line_break_info>;
using unicode_line_break_info_iterator = unicode_line_break_info_vector::iterator;
using unicode_line_break_info_const_iterator = unicode_line_break_info_vector::const_iterator;

template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_info>
unicode_LB1(It first, ItEnd last, DescriptionFunc const &description_func) noexcept
{
    auto r = std::vector<unicode_line_break_info>{};
    r.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it) {
        hilet &description = description_func(*it);
        hilet break_class = description.line_break_class();
        hilet general_category = description.general_category();

        hilet resolved_break_class = [&]() {
            switch (break_class) {
                using enum unicode_line_break_class;
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
            general_category == unicode_general_category::Cn,
            description.grapheme_cluster_break() == unicode_grapheme_cluster_break::Extended_Pictographic,
            description.east_asian_width());
    }

    return r;
}

[[nodiscard]] constexpr void unicode_LB2_3(unicode_break_vector &opportunities) noexcept
{
    hi_axiom(not opportunities.empty());
    // LB2
    opportunities.front() = unicode_break_opportunity::no;
    // LB3
    opportunities.back() = unicode_break_opportunity::mandatory;
}

template<typename MatchFunc>
constexpr void unicode_LB_walk(
    unicode_break_vector &opportunities,
    std::vector<unicode_line_break_info> const &infos,
    MatchFunc match_func) noexcept
{
    using enum unicode_line_break_class;

    if (infos.empty()) {
        return;
    }

    auto cur = infos.begin();
    hilet last = infos.end() - 1;
    hilet last2 = infos.end();
    auto opportunity = opportunities.begin() + 1;

    auto cur_sp_class = XX;
    auto cur_nu_class = XX;
    auto prev_class = XX;
    auto num_ri = 0_uz;
    while (cur != last) {
        hilet next = cur + 1;
        hilet cur_class = unicode_line_break_class{*cur};
        hilet next2_class = cur + 2 == last2 ? XX : unicode_line_break_class{*(cur + 2)};

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

        if (*opportunity == unicode_break_opportunity::unassigned) {
            *opportunity = match_func(prev_class, cur, next, next2_class, cur_sp_class, cur_nu_class, num_ri);
        }

        prev_class = cur_class;
        cur = next;
        ++opportunity;
    }
}

constexpr void unicode_LB4_8a(unicode_break_vector &opportunities, std::vector<unicode_line_break_info> const &infos) noexcept
{
    unicode_LB_walk(
        opportunities, infos, [](hilet prev, hilet cur, hilet next, hilet next2, hilet cur_sp, hilet cur_nu, hilet num_ri) {
            using enum unicode_break_opportunity;
            using enum unicode_line_break_class;
            if (*cur == BK) {
                return mandatory; // LB4: 4.0
            } else if (*cur == CR and *next == LF) {
                return no; // LB5: 5.01
            } else if (*cur == CR or *cur == LF or *cur == NL) {
                return mandatory; // LB5: 5.02, 5.03, 5.04
            } else if (*next == BK or *next == CR or *next == LF or *next == NL) {
                return no; // LB6: 6.0
            } else if (*next == SP or *next == ZW) {
                return no; // LB7: 7.01, 7.02
            } else if (cur_sp == ZW) {
                return yes; // LB8: 8.0
            } else if (*cur == ZWJ) {
                return no; // LB8a: 8.1
            } else {
                return unassigned;
            }
        });
}

constexpr void unicode_LB9(unicode_break_vector &opportunities, std::vector<unicode_line_break_info> &infos) noexcept
{
    using enum unicode_line_break_class;
    using enum unicode_break_opportunity;

    if (infos.empty()) {
        return;
    }

    auto cur = infos.begin();
    hilet last = infos.end() - 1;
    auto opportunity = opportunities.begin() + 1;

    auto X = XX;
    while (cur != last) {
        hilet next = cur + 1;

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
            *opportunity = no;

            if (X == XX) {
                // The first character of [^BK CR LF NL SP ZW] x [CM ZWJ]* => X
                X = static_cast<unicode_line_break_class>(*cur);
            }
        }

        cur = next;
        ++opportunity;
    }
}

constexpr void unicode_LB10(std::vector<unicode_line_break_info> &infos) noexcept
{
    using enum unicode_line_break_class;

    for (auto &x : infos) {
        if (x == CM or x == ZWJ) {
            x |= AL;
        }
    }
}

constexpr void unicode_LB11_31(unicode_break_vector &opportunities, std::vector<unicode_line_break_info> const &infos) noexcept
{
    unicode_LB_walk(
        opportunities, infos, [&](hilet prev, hilet cur, hilet next, hilet next2, hilet cur_sp, hilet cur_nu, hilet num_ri) {
            using enum unicode_break_opportunity;
            using enum unicode_line_break_class;
            using enum unicode_east_asian_width;

            if (*cur == WJ or *next == WJ) {
                return no; // LB11: 11.01, 11.02
            } else if (*cur == GL) {
                return no; // LB12: 12.0
            } else if (*cur != SP and *cur != BA and *cur != HY and *next == GL) {
                return no; // LB12a: 12.1
            } else if (*next == CL or *next == CP or *next == EX or *next == IS or *next == SY) {
                return no; // LB13: 13.0
            } else if (cur_sp == OP) {
                return no; // LB14: 14.0
            } else if (cur_sp == QU and *next == OP) {
                return no; // LB15: 15.0
            } else if ((cur_sp == CL or cur_sp == CP) and *next == NS) {
                return no; // LB16: 16.0
            } else if (cur_sp == B2 and *next == B2) {
                return no; // LB17: 17.0
            } else if (*cur == SP) {
                return yes; // LB18: 18.0
            } else if (*cur == QU or *next == QU) {
                return no; // LB19: 19.01, 19.02
            } else if (*cur == CB or *next == CB) {
                return yes; // LB20: 20.01, 20.02
            } else if (*cur == BB or *next == BA or *next == HY or *next == NS) {
                return no; // LB21: 21.01, 21.02, 21.03, 21.04
            } else if (prev == HL and (*cur == HY or *cur == BA)) {
                return no; // LB21a: 21.1
            } else if (*cur == SY and *next == HL) {
                return no; // LB21b: 21.2
            } else if (*next == IN) {
                return no; // LB22: 22.0
            } else if ((*cur == AL or *cur == HL) and *next == NU) {
                return no; // LB23: 23.02
            } else if (*cur == NU and (*next == AL or *next == HL)) {
                return no; // LB23: 23.03
            } else if (*cur == PR and (*next == ID or *next == EB or *next == EM)) {
                return no; // LB23a: 23.12
            } else if ((*cur == ID or *cur == EB or *cur == EM) and *next == PO) {
                return no; // LB23a: 23.13
            } else if ((*cur == PR or *cur == PO) and (*next == AL or *next == HL)) {
                return no; // LB24: 24.02
            } else if ((*cur == AL or *cur == HL) and (*next == PR or *next == PO)) {
                return no; // LB24: 24.03
            } else if (
                (*cur == PR or *cur == PO) and ((*next == OP and next2 == NU) or (*next == HY and next2 == NU) or *next == NU)) {
                return no; // LB25: 25.01
            } else if ((*cur == OP or *cur == HY) and *next == NU) {
                return no; // LB25: 25.02
            } else if (*cur == NU and (*next == NU or *next == SY or *next == IS)) {
                return no; // LB25: 25.03
            } else if (cur_nu == NU and (*next == NU or *next == SY or *next == IS or *next == CL or *next == CP)) {
                return no; // LB25: 25.04
            } else if ((cur_nu == NU or cur_nu == CL) and (*next == PO or *next == PR)) {
                return no; // LB25: 25.05
            } else if (*cur == JL and (*next == JL or *next == JV or *next == H2 or *next == H3)) {
                return no; // LB26: 26.01
            } else if ((*cur == JV or *cur == H2) and (*next == JV or *next == JT)) {
                return no; // LB26: 26.02
            } else if ((*cur == JT or *cur == H3) and *next == JT) {
                return no; // LB26: 26.03
            } else if ((*cur == JL or *cur == JV or *cur == JT or *cur == H2 or *cur == H3) and *next == PO) {
                return no; // LB27: 27.01
            } else if (*cur == PR and (*next == JL or *next == JV or *next == JT or *next == H2 or *next == H3)) {
                return no; // LB27: 27.02
            } else if ((*cur == AL or *cur == HL) and (*next == AL or *next == HL)) {
                return no; // LB28: 28.0
            } else if (*cur == IS and (*next == AL or *next == HL)) {
                return no; // LB29: 29.0
            } else if ((*cur == AL or *cur == HL or *cur == NU) and (*next == OP and *next != F and *next != W and *next != H)) {
                return no; // LB30: 30.01
            } else if ((*cur == CP and *cur != F and *cur != W and *cur != H) and (*next == AL or *next == HL or *next == NU)) {
                return no; // LB30: 30.02
            } else if (*cur == RI and *next == RI and (num_ri % 2) == 1) {
                return no; // LB30a: 30.11, 30.12, 30.13
            } else if (*cur == EB and *next == EM) {
                return no; // LB30b: 30.21
            } else if (cur->is_extended_pictographic and cur->is_Cn and *next == EM) {
                return no; // LB30b: 30.22
            } else {
                return yes; // LB31: 999.0
            }
        });
}

[[nodiscard]] constexpr bool unicode_LB_width_check(
    std::vector<float> const &widths,
    std::vector<size_t> const &lengths,
    float maximum_line_width) noexcept
{
    auto it = widths.begin();
    for (auto length : lengths) {
        if (unicode_line_break_width(it, it + length) > maximum_line_width) {
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
[[nodiscard]] constexpr std::vector<size_t> unicode_LB_mandatory_lines(unicode_break_vector const &opportunities) noexcept
{
    auto r = std::vector<size_t>{};

    auto length = 0_uz;
    for (auto it = opportunities.begin() + 1; it != opportunities.end(); ++it) {
        ++length;
        if (*it == unicode_break_opportunity::mandatory) {
            r.push_back(length);
            length = 0_uz;
        }
    }

    return r;
}

[[nodiscard]] constexpr unicode_break_const_iterator unicode_LB_fast_fit_line(
    unicode_break_const_iterator opportunity_it,
    std::vector<float>::const_iterator width_it,
    float maximum_line_width) noexcept
{
    using enum unicode_break_opportunity;

    auto width = 0.0f;
    auto end_of_line = opportunity_it;
    while (true) {
        width += abs(*width_it);
        if (width > maximum_line_width) {
            // This character makes the width too long.
            return end_of_line;

        } else if (*opportunity_it == mandatory) {
            // This character is an end-of-line.
            return opportunity_it;

        } else if (*opportunity_it == yes) {
            // This character is a valid break opportunity.
            end_of_line = opportunity_it;
        }

        ++opportunity_it;
        ++width_it;
    }
    hi_unreachable();
}

[[nodiscard]] constexpr unicode_break_const_iterator unicode_LB_slow_fit_line(
    unicode_break_const_iterator first,
    unicode_break_const_iterator end_of_line,
    std::vector<float>::const_iterator first_width,
    float maximum_line_width) noexcept
{
    using enum unicode_break_opportunity;

    // Carefully look forward for a break opportunity.
    auto it = end_of_line;
    while (true) {
        hilet num_characters = std::distance(first, it + 1);
        hilet line_width = unicode_line_break_width(first_width, first_width + num_characters);

        if (line_width <= maximum_line_width) {
            if (*it == mandatory) {
                // The next mandatory break fits in the maximum width.
                return it;

            } else if (*it == yes) {
                // The next break opportunity fits in the maximum width.
                end_of_line = it;
            }
        } else {
            // This break opportunity doesn't fit within the maximum width. Use the previous break opportunity.
            return end_of_line;
        }

        ++it;
    }
    hi_unreachable();
}

[[nodiscard]] constexpr unicode_break_const_iterator
unicode_LB_finish_fit_line(unicode_break_const_iterator first, unicode_break_const_iterator end_of_line) noexcept
{
    if (first == end_of_line) {
        // We couldn't break the line to fit the maximum line width.
        while (*end_of_line == unicode_break_opportunity::no) {
            ++end_of_line;
        }
    }

    // Return iterator past the end-of-line.
    return end_of_line + 1;
}

/** Get the length of each line when broken with mandatory breaks.
 *
 * @return A list of line lengths.
 */
[[nodiscard]] constexpr std::vector<size_t> unicode_LB_fit_lines(
    unicode_break_vector const &opportunities,
    std::vector<float> const &widths,
    float maximum_line_width) noexcept
{
    using enum unicode_break_opportunity;

    auto r = std::vector<size_t>{};
    if (widths.empty()) {
        return r;
    }

    auto opportunity_it = opportunities.begin() + 1;
    auto width_it = widths.begin();
    while (width_it != widths.end()) {
        // First quickly find when the line is too long.
        auto opportunity_eol = unicode_LB_fast_fit_line(opportunity_it, width_it, maximum_line_width);
        opportunity_eol = unicode_LB_slow_fit_line(opportunity_it, opportunity_eol, width_it, maximum_line_width);
        opportunity_eol = unicode_LB_finish_fit_line(opportunity_it, opportunity_eol);

        hilet num_characters = std::distance(opportunity_it, opportunity_eol);
        r.push_back(num_characters);
        opportunity_it += num_characters;
        width_it += num_characters;
    }

    return r;
}

} // namespace detail

/** The unicode line break algorithm UAX #14
 *
 * @param first An iterator to the first character.
 * @param last An iterator to the last character.
 * @param description_func A function to get a reference to unicode_description from a character.
 * @return A list of unicode_break_opportunity.
 */
template<typename It, typename ItEnd, typename DescriptionFunc>
[[nodiscard]] inline unicode_break_vector unicode_line_break(It first, ItEnd last, DescriptionFunc const &description_func) noexcept
{
    auto size = narrow_cast<size_t>(std::distance(first, last));
    auto r = unicode_break_vector{size + 1, unicode_break_opportunity::unassigned};

    auto infos = detail::unicode_LB1(first, last, description_func);
    detail::unicode_LB2_3(r);
    detail::unicode_LB4_8a(r, infos);
    detail::unicode_LB9(r, infos);
    detail::unicode_LB10(infos);
    detail::unicode_LB11_31(r, infos);
    return r;
}

/** Unicode break lines.
 *
 * @param opportunities The list of break opportunities.
 * @param widths The list of character widths
 * @param maximum_line_width The maximum line width.
 * @return A list of line lengths.
 */
[[nodiscard]] constexpr std::vector<size_t>
unicode_line_break(unicode_break_vector const &opportunities, std::vector<float> const &widths, float maximum_line_width)
{
    // See if the lines after mandatory breaks will fit the width and return.
    auto r = detail::unicode_LB_mandatory_lines(opportunities);
    if (detail::unicode_LB_width_check(widths, r, maximum_line_width)) {
        return r;
    }

    r = detail::unicode_LB_fit_lines(opportunities, widths, maximum_line_width);
    hi_axiom(detail::unicode_LB_width_check(widths, r, maximum_line_width));
    return r;
}

} // namespace hi::inline v1
