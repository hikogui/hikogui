// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>
#include <vector>

export module hikogui_unicode_unicode_grapheme_cluster_break;
import hikogui_unicode_ucd_grapheme_cluster_breaks;
import hikogui_unicode_unicode_break_opportunity;

export namespace hi { inline namespace v1 {
namespace detail {

struct grapheme_break_state {
    unicode_grapheme_cluster_break previous = unicode_grapheme_cluster_break::Other;
    int RI_count = 0;
    bool first_character = true;
    bool in_extended_pictograph = false;

    constexpr void reset() noexcept
    {
        previous = unicode_grapheme_cluster_break::Other;
        RI_count = 0;
        first_character = true;
        in_extended_pictograph = false;
    }
};

/**
 * This function implements the "Grapheme Cluster Boundary Rules" described in:
 * https://www.unicode.org/reports/tr29/tr29-41.html#Grapheme_Cluster_Boundary_Rules
 *
 * @param cluster_break The grapheme cluster to check for break opportunities.
 * @param state The grapheme break state for tracking the break opportunity.
 * @return True, if the grapheme is breakable.
 */
[[nodiscard]] constexpr bool breaks_grapheme(unicode_grapheme_cluster_break cluster_break, grapheme_break_state& state) noexcept
{
    using enum unicode_grapheme_cluster_break;

    hilet lhs = state.previous;
    hilet rhs = cluster_break;

    enum class break_state {
        unknown,
        do_break,
        dont_break,
    };

    break_state break_state = break_state::unknown;

    // GB1, GB2: Break at the start and end of text, unless the text is empty.
    bool GB1 = state.first_character;
    if ((break_state == break_state::unknown) & GB1) {
        break_state = break_state::do_break;
    }

    state.first_character = false;

    // GB3, GB4, GB5: Do not break between a CR and LF. Otherwise, break before and after controls.
    hilet GB3 = (lhs == CR) && (rhs == LF);
    hilet GB4 = (lhs == Control) || (lhs == CR) || (lhs == LF);
    hilet GB5 = (rhs == Control) || (rhs == CR) || (rhs == LF);
    if (break_state == break_state::unknown) {
        if (GB3) {
            break_state = break_state::dont_break;
        } else if (GB4 || GB5) {
            break_state = break_state::do_break;
        }
    }

    // GB6, GB7, GB8: Do not break Hangul syllable sequences.
    hilet GB6 = (lhs == L) && ((rhs == L) || (rhs == V) || (rhs == LV) | (rhs == LVT));
    hilet GB7 = ((lhs == LV) || (lhs == V)) && ((rhs == V) || (rhs == T));
    hilet GB8 = ((lhs == LVT) || (lhs == T)) && (rhs == T);
    if ((break_state == break_state::unknown) && (GB6 || GB7 || GB8)) {
        break_state = break_state::dont_break;
    }

    // GB9: Do not break before extending characters or ZWJ.
    hilet GB9 = ((rhs == Extend) || (rhs == ZWJ));

    // GB9a, GB9b: Do not break before SpacingMarks, or after Prepend characters.
    // Both rules only apply to extended grapheme clusters.
    hilet GB9a = (rhs == SpacingMark);
    hilet GB9b = (lhs == Prepend);
    if ((break_state == break_state::unknown) & (GB9 || GB9a || GB9b)) {
        break_state = break_state::dont_break;
    }

    // GB11: Do not break within emoji modifier sequences or emoji zwj sequences.
    hilet GB11 = state.in_extended_pictograph && (lhs == ZWJ) && (rhs == Extended_Pictographic);
    if ((break_state == break_state::unknown) && GB11) {
        break_state = break_state::dont_break;
    }

    if (rhs == Extended_Pictographic) {
        state.in_extended_pictograph = true;
    } else if (!((rhs == Extend) || (rhs == ZWJ))) {
        state.in_extended_pictograph = false;
    }

    // GB12, GB13: Do not break within emoji flag sequences.
    // That is, do not break between regional indicator (RI) symbols,
    // if there is an odd number of RI characters before the break point.
    hilet GB12_13 = (lhs == Regional_Indicator) && (rhs == Regional_Indicator) && ((state.RI_count % 2) == 1);
    if ((break_state == break_state::unknown) && (GB12_13)) {
        break_state = break_state::dont_break;
    }

    if (rhs == Regional_Indicator) {
        state.RI_count++;
    } else {
        state.RI_count = 0;
    }

    // GB999: Otherwise, break everywhere.
    if (break_state == break_state::unknown) {
        break_state = break_state::do_break;
    }

    state.previous = rhs;
    return break_state == break_state::do_break;
}

/** Check if for a grapheme break before the given code-point.
 * Code points must be tested in order, starting at the beginning of the text.
 *
 * @param code_point Current code point to test.
 * @param state Current state of the grapheme-break algorithm.
 * @return true when a grapheme break exists before the current code-point.
 */
[[nodiscard]] constexpr bool breaks_grapheme(char32_t code_point, grapheme_break_state& state) noexcept
{
    return breaks_grapheme(ucd_get_grapheme_cluster_break(code_point), state);
}

} // namespace detail

template<typename It, typename ItEnd>
[[nodiscard]] constexpr std::vector<unicode_break_opportunity> unicode_grapheme_break(It first, ItEnd last) noexcept
{
    auto r = std::vector<unicode_break_opportunity>{};
    auto state = detail::grapheme_break_state{};

    for (auto it = first; it != last; ++it) {
        hilet opportunity = detail::breaks_grapheme(*it, state) ? unicode_break_opportunity::yes : unicode_break_opportunity::no;
        r.push_back(opportunity);
    }

    r.push_back(unicode_break_opportunity::yes);
    return r;
}

}} // namespace hi::v1
