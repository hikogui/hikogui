// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file unicode/unicode_line_break.hpp
 */

#pragma once

#include "unicode_break_opportunity.hpp"
#include "ucd_general_categories.hpp"
#include "ucd_grapheme_cluster_breaks.hpp"
#include "ucd_line_break_classes.hpp"
#include "ucd_east_asian_widths.hpp"
#include "../algorithm/algorithm.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ranges>

hi_export_module(hikogui.unicode.unicode_line_break);

hi_export namespace hi::inline v1 {
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
    constexpr unicode_line_break_info(unicode_line_break_info const&) noexcept = default;
    constexpr unicode_line_break_info(unicode_line_break_info&&) noexcept = default;
    constexpr unicode_line_break_info& operator=(unicode_line_break_info const&) noexcept = default;
    constexpr unicode_line_break_info& operator=(unicode_line_break_info&&) noexcept = default;

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

    constexpr unicode_line_break_info& operator|=(unicode_line_break_class rhs) noexcept
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

template<typename It, typename ItEnd, typename CodePointFunc>
[[nodiscard]] constexpr std::vector<unicode_line_break_info>
unicode_LB1(It first, ItEnd last, CodePointFunc const& code_point_func) noexcept
{
    auto r = std::vector<unicode_line_break_info>{};
    r.reserve(std::distance(first, last));

    for (auto it = first; it != last; ++it) {
        auto const code_point = code_point_func(*it);
        auto const east_asian_width = ucd_get_east_asian_width(code_point);
        auto const break_class = ucd_get_line_break_class(code_point);
        auto const general_category = ucd_get_general_category(code_point);
        auto const grapheme_cluster_break = ucd_get_grapheme_cluster_break(code_point);

        auto const resolved_break_class = [&]() {
            switch (break_class) {
                using enum unicode_line_break_class;
            case AI:
            case SG:
            case XX:
                return AL;
            case CJ:
                return NS;
            case SA:
                return is_Mn_or_Mc(general_category) ? CM : AL;
            default:
                return break_class;
            }
        }();

        r.emplace_back(
            resolved_break_class,
            general_category == unicode_general_category::Cn,
            grapheme_cluster_break == unicode_grapheme_cluster_break::Extended_Pictographic,
            east_asian_width);
    }

    return r;
}

constexpr void unicode_LB2_3(unicode_line_break_vector& opportunities) noexcept
{
    hi_axiom(not opportunities.empty());
    // LB2
    opportunities.front() = unicode_break_opportunity::no;
    // LB3
    opportunities.back() = unicode_break_opportunity::mandatory;
}

template<typename MatchFunc>
constexpr void unicode_LB_walk(
    unicode_line_break_vector& opportunities,
    std::vector<unicode_line_break_info> const& infos,
    MatchFunc match_func) noexcept
{
    using enum unicode_line_break_class;

    if (infos.empty()) {
        return;
    }

    auto cur = infos.begin();
    auto const last = infos.end() - 1;
    auto const last2 = infos.end();
    auto opportunity = opportunities.begin() + 1;

    auto cur_sp_class = XX;
    auto cur_nu_class = XX;
    auto prev_class = XX;
    auto num_ri = 0_uz;
    while (cur != last) {
        auto const next = cur + 1;
        auto const cur_class = unicode_line_break_class{*cur};
        auto const next2_class = cur + 2 == last2 ? XX : unicode_line_break_class{*(cur + 2)};

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

constexpr void
unicode_LB4_8a(unicode_line_break_vector& opportunities, std::vector<unicode_line_break_info> const& infos) noexcept
{
    unicode_LB_walk(
        opportunities,
        infos,
        [](auto const prev,
           auto const cur,
           auto const next,
           auto const next2,
           auto const cur_sp,
           auto const cur_nu,
           auto const num_ri) {
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

constexpr void unicode_LB9(unicode_line_break_vector& opportunities, std::vector<unicode_line_break_info>& infos) noexcept
{
    using enum unicode_line_break_class;
    using enum unicode_break_opportunity;

    if (infos.empty()) {
        return;
    }

    auto cur = infos.begin();
    auto const last = infos.end() - 1;
    auto opportunity = opportunities.begin() + 1;

    auto X = XX;
    while (cur != last) {
        auto const next = cur + 1;

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

constexpr void unicode_LB10(std::vector<unicode_line_break_info>& infos) noexcept
{
    using enum unicode_line_break_class;

    for (auto& x : infos) {
        if (x == CM or x == ZWJ) {
            x |= AL;
        }
    }
}

constexpr void
unicode_LB11_31(unicode_line_break_vector& opportunities, std::vector<unicode_line_break_info> const& infos) noexcept
{
    unicode_LB_walk(
        opportunities,
        infos,
        [&](auto const prev,
            auto const cur,
            auto const next,
            auto const next2,
            auto const cur_sp,
            auto const cur_nu,
            auto const num_ri) {
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

/** Calculate the width of a line.
 *
 * @param first Iterator to the first character widths.
 * @param last Iterator to one beyond the last character width.
 * @return The length of the line.
 */
template<
    std::random_access_iterator It,
    std::invocable<std::iter_value_t<It>> AdvanceOp,
    typename WhitespaceOp>
[[nodiscard]] constexpr std::invoke_result_t<AdvanceOp, std::iter_value_t<It>> unicode_LB_width(
    It first,
    It last,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op) noexcept requires std::is_invocable_r_v<bool, WhitespaceOp, std::iter_value_t<It>>
{
    using width_type = std::invoke_result_t<AdvanceOp, std::iter_value_t<It>>;

    auto last_visible_it = rfind_if_not(first, last, whitespace_op);
    if (last_visible_it == last) {
        // All characters are whitespace.
        return width_type{};
    }

    last = last_visible_it + 1;
    return std::accumulate(first, last, width_type{}, [&](auto acc, auto x) {
        return acc + advance_op(x);
    });
}

/** Get the width of the entire text.
 *
 * @param widths Width of each character in the text.
 * @param lengths Number of characters on each line.
 * @return The maximum line width.
 */
template<
    std::ranges::random_access_range GraphemeInfoRange,
    std::invocable<std::ranges::range_value_t<GraphemeInfoRange>> AdvanceOp,
    typename WhitespaceOp>
[[nodiscard]] constexpr std::invoke_result_t<AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>> unicode_LB_width(
    GraphemeInfoRange const& grapheme_metrics_range,
    std::vector<size_t> const& lengths,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op) noexcept
    requires std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    using width_type = std::invoke_result_t<AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>>;

    auto max_width = width_type{};
    auto it = std::ranges::begin(grapheme_metrics_range);
    for (auto length : lengths) {
        inplace_max(max_width, unicode_LB_width(it, it + length, advance_op, whitespace_op));
        it += length;
    }
    return max_width;
}

/** Check if all the lines in the text fit the maximum width.
 *
 * @param widths Width of each character in the text.
 * @param lengths Number of characters on each line.
 * @param maximum_line_width The maximum line width allowed.
 * @param advance_op A function returning the advance of a grapheme.
 * @param whitespace_op A function returning if the grapheme is a whitespace
 *                      character.
 * @return True if all the lines fit the maximum width.
 */
template<std::ranges::random_access_range GraphemeInfoRange, typename MaximumLineWidth, typename AdvanceOp, typename WhitespaceOp>
[[nodiscard]] constexpr bool unicode_LB_width_check(
    GraphemeInfoRange const& grapheme_info,
    std::vector<size_t> const& lengths,
    MaximumLineWidth maximum_line_width,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op) noexcept
    requires std::is_invocable_r_v<MaximumLineWidth, AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>> and
    std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    auto it = grapheme_info.begin();
    for (auto length : lengths) {
        if (unicode_LB_width(it, it + length, advance_op, whitespace_op) > maximum_line_width) {
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
[[nodiscard]] constexpr std::vector<size_t> unicode_LB_mandatory_lines(unicode_line_break_vector const& opportunities) noexcept
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

/** Get the length of each line when broken with mandatory and optional breaks.
 *
 * @return A list of line lengths.
 */
[[nodiscard]] constexpr std::vector<size_t> unicode_LB_optional_lines(unicode_line_break_vector const& opportunities) noexcept
{
    auto r = std::vector<size_t>{};

    auto length = 0_uz;
    for (auto it = opportunities.begin() + 1; it != opportunities.end(); ++it) {
        ++length;
        if (*it != unicode_break_opportunity::no) {
            r.push_back(length);
            length = 0_uz;
        }
    }

    return r;
}

template<std::random_access_iterator GraphemeInfoIt, typename Width, typename AdvanceOp>
[[nodiscard]] constexpr unicode_line_break_vector::const_iterator unicode_LB_fast_fit_line(
    unicode_line_break_vector::const_iterator opportunity_it,
    GraphemeInfoIt grapheme_info_it,
    Width maximum_line_width,
    AdvanceOp const& advance_op) noexcept
    requires std::is_invocable_r_v<Width, AdvanceOp, std::iter_value_t<GraphemeInfoIt>>
{
    using enum unicode_break_opportunity;

    auto width = Width{};
    auto end_of_line = opportunity_it;
    while (true) {
        width += advance_op(*grapheme_info_it);
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
        ++grapheme_info_it;
    }
    std::unreachable();
}

template<std::random_access_iterator GraphemeInfoIt, typename Width, typename AdvanceOp, typename WhitespaceOp>
[[nodiscard]] constexpr unicode_line_break_vector::const_iterator unicode_LB_slow_fit_line(
    unicode_line_break_vector::const_iterator first,
    unicode_line_break_vector::const_iterator end_of_line,
    GraphemeInfoIt grapheme_info_it,
    Width maximum_line_width,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op) noexcept
    requires std::is_invocable_r_v<Width, AdvanceOp, std::iter_value_t<GraphemeInfoIt>> and
    std::is_invocable_r_v<bool, WhitespaceOp, std::iter_value_t<GraphemeInfoIt>>
{
    using enum unicode_break_opportunity;

    // Carefully look forward for a break opportunity.
    auto it = end_of_line;
    while (true) {
        auto const num_graphemes = std::distance(first, it + 1);
        auto const line_width = unicode_LB_width(grapheme_info_it, grapheme_info_it + num_graphemes, advance_op, whitespace_op);

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
    std::unreachable();
}

[[nodiscard]] constexpr unicode_line_break_vector::const_iterator unicode_LB_finish_fit_line(
    unicode_line_break_vector::const_iterator first,
    unicode_line_break_vector::const_iterator end_of_line) noexcept
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

/** Get the length of each line when broken after folding text to a maximum width.
 *
 * @return A list of line lengths.
 */
template<std::ranges::random_access_range GraphemeInfoRange, typename Width, typename AdvanceOp, typename WhitespaceOp>
[[nodiscard]] constexpr std::vector<size_t> unicode_LB_fit_lines(
    unicode_line_break_vector const& opportunities,
    GraphemeInfoRange const& grapheme_metrics_range,
    Width maximum_line_width,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op)
    requires std::is_invocable_r_v<Width, AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>> and
    std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    auto r = std::vector<size_t>{};
    if (grapheme_metrics_range.empty()) {
        return r;
    }

    auto opportunity_it = opportunities.begin() + 1;
    auto grapheme_info_it = std::ranges::begin(grapheme_metrics_range);
    while (grapheme_info_it != std::ranges::end(grapheme_metrics_range)) {
        // First quickly find when the line is too long.
        auto opportunity_eol = unicode_LB_fast_fit_line(opportunity_it, grapheme_info_it, maximum_line_width, advance_op);
        opportunity_eol = unicode_LB_slow_fit_line(
            opportunity_it, opportunity_eol, grapheme_info_it, maximum_line_width, advance_op, whitespace_op);
        opportunity_eol = unicode_LB_finish_fit_line(opportunity_it, opportunity_eol);

        auto const num_graphemes = std::distance(opportunity_it, opportunity_eol);
        r.push_back(num_graphemes);
        opportunity_it += num_graphemes;
        grapheme_info_it += num_graphemes;
    }

    return r;
}

/** Get the maximum width of the text.
 *
 * The width of the text when using only the mandatory break-opportunity.
 *
 * @param opportunities The break-opportunity per character.
 * @param char_widths The width of each character.
 * @return The maximum line width of the text, number of lines.
 */
template<
    std::ranges::random_access_range GraphemeInfoRange,
    std::invocable<std::ranges::range_value_t<GraphemeInfoRange>> AdvanceOp,
    typename WhitespaceOp>
[[nodiscard]] constexpr std::
    pair<std::invoke_result_t<AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>>, std::vector<size_t>>
    unicode_LB_maximum_width(
        unicode_line_break_vector const& break_opportunities,
        GraphemeInfoRange const& grapheme_metrics_range,
        AdvanceOp const& advance_op,
        WhitespaceOp const& whitespace_op)
        requires std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>

{
    auto const line_lengths = detail::unicode_LB_mandatory_lines(break_opportunities);
    auto width = detail::unicode_LB_width(grapheme_metrics_range, line_lengths, advance_op, whitespace_op);
    return {width, std::move(line_lengths)};
}

/** Get the minimum width of the text.
 *
 * The width of the text when using each and every break-opportunity.
 *
 * @param opportunities The break-opportunity per character.
 * @param char_widths The width of each character.
 * @return The minimum line width of the text, number of lines.
 */
template<
    std::ranges::random_access_range GraphemeInfoRange,
    std::invocable<std::ranges::range_value_t<GraphemeInfoRange>> AdvanceOp,
    typename WhitespaceOp>
[[nodiscard]] constexpr std::
    pair<std::invoke_result_t<AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>>, std::vector<size_t>>
    unicode_LB_minimum_width(
        unicode_line_break_vector const& break_opportunities,
        GraphemeInfoRange const& grapheme_metrics_range,
        AdvanceOp const& advance_op,
        WhitespaceOp const& whitespace_op)
        requires std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    auto const line_lengths = detail::unicode_LB_optional_lines(break_opportunities);
    auto width = detail::unicode_LB_width(grapheme_metrics_range, line_lengths, advance_op, whitespace_op);
    return {width, std::move(line_lengths)};
}

/** Get the width of the text at a maximum width.
 *
 * The width of the text when folding the text to a maximum width.
 *
 * @param opportunities The break-opportunity per character.
 * @param char_widths The width of each character.
 * @param maximum_line_width The maximum width of the text
 * @return The minimum line width of the folded text, number of lines.
 */
template<
    std::ranges::random_access_range GraphemeInfoRange,
    typename Width,
    typename AdvanceOp,
    typename WhitespaceOp>
[[nodiscard]] constexpr std::pair<Width, std::vector<size_t>>
unicode_LB_width(unicode_line_break_vector const& opportunities, GraphemeInfoRange const& grapheme_metrics_range, Width maximum_line_width, AdvanceOp const& advance_op, WhitespaceOp const& whitespace_op)
requires std::is_invocable_r_v<Width, AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>> and
    std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    auto const line_lengths = detail::unicode_LB_fit_lines(opportunities, grapheme_metrics_range, maximum_line_width, advance_op, whitespace_op);
    auto width = detail::unicode_LB_width(grapheme_metrics_range, line_lengths, advance_op, whitespace_op);
    return {width, std::move(line_lengths)};
}

} // namespace detail

/** The unicode line break algorithm UAX #14
 *
 * @param first An iterator to the first character.
 * @param last An iterator to the last character.
 * @param code_point_func A function to get the code-point of a character.
 * @return A list of unicode_break_opportunity.
 */
template<typename It, typename ItEnd, typename CodePointFunc>
[[nodiscard]] inline unicode_line_break_vector
unicode_line_break(It first, ItEnd last, CodePointFunc const& code_point_func) noexcept
{
    auto size = narrow_cast<size_t>(std::distance(first, last));
    auto r = unicode_line_break_vector{size + 1, unicode_break_opportunity::unassigned};

    auto infos = detail::unicode_LB1(first, last, code_point_func);
    detail::unicode_LB2_3(r);
    detail::unicode_LB4_8a(r, infos);
    detail::unicode_LB9(r, infos);
    detail::unicode_LB10(infos);
    detail::unicode_LB11_31(r, infos);
    return r;
}

[[nodiscard]] inline unicode_line_break_vector unicode_line_break(gstring_view text) noexcept
{
    return unicode_line_break(text.begin(), text.end(), [](auto c) {
        return c.starter();
    });
}

/** Unicode break lines.
 *
 * It may be possible that the lines is still longer than the maximum width.
 * For example when a line has no break opportunities, such as a single word
 * and the word is too long.
 *
 * @param opportunities The list of break opportunities.
 * @param graphemes_info The list of grapheme information, which includes the
 *                       advance of each grapheme, and if the grapheme is a
 *                       whitespace character.
 * @param maximum_line_width The maximum line width.
 * @param advance_op A function returning the advance of a grapheme.
 * @param whitespace_op A function returning if the grapheme is a whitespace
 *                      character.
 * @return A list of line lengths.
 */
template<std::ranges::input_range GraphemeInfoRange, typename MaximumLineWidth, typename AdvanceOp, typename WhitespaceOp>
[[nodiscard]] constexpr std::vector<size_t> unicode_fold_lines(
    unicode_line_break_vector const& opportunities,
    GraphemeInfoRange const& graphemes_info,
    MaximumLineWidth maximum_line_width,
    AdvanceOp const& advance_op,
    WhitespaceOp const& whitespace_op)
    requires std::is_invocable_r_v<MaximumLineWidth, AdvanceOp, std::ranges::range_value_t<GraphemeInfoRange>> and
    std::is_invocable_r_v<bool, WhitespaceOp, std::ranges::range_value_t<GraphemeInfoRange>>
{
    // See if the lines after mandatory breaks will fit the width and return.
    auto r = detail::unicode_LB_mandatory_lines(opportunities);
    if (detail::unicode_LB_width_check(graphemes_info, r, maximum_line_width, advance_op, whitespace_op)) {
        return r;
    }

    r = detail::unicode_LB_fit_lines(opportunities, graphemes_info, maximum_line_width, advance_op, whitespace_op);
    return r;
}

} // namespace hi::inline v1
