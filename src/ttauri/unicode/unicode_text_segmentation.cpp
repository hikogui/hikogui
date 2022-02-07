// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_text_segmentation.hpp"
#include "unicode_description.hpp"
#include "../required.hpp"

namespace tt::inline v1 {

[[nodiscard]] static bool breaks_grapheme(unicode_grapheme_cluster_break cluster_break, grapheme_break_state &state) noexcept
{
    using enum unicode_grapheme_cluster_break;

    ttlet lhs = state.previous;
    ttlet rhs = cluster_break;

    enum class break_state {
        unknown,
        do_break,
        dont_break,
    };

    break_state break_state = break_state::unknown;

    bool GB1 = state.first_character;
    if ((break_state == break_state::unknown) & GB1) {
        break_state = break_state::do_break;
    }

    state.first_character = false;

    ttlet GB3 = (lhs == CR) && (rhs == LF);
    ttlet GB4 = (lhs == Control) || (lhs == CR) || (lhs == LF);
    ttlet GB5 = (rhs == Control) || (rhs == CR) || (rhs == LF);
    if (break_state == break_state::unknown) {
        if (GB3) {
            break_state = break_state::dont_break;
        } else if (GB4 || GB5) {
            break_state = break_state::do_break;
        }
    }

    ttlet GB6 = (lhs == L) && ((rhs == L) || (rhs == V) || (rhs == LV) | (rhs == LVT));
    ttlet GB7 = ((lhs == LV) || (lhs == V)) && ((rhs == V) || (rhs == T));
    ttlet GB8 = ((lhs == LVT) || (lhs == T)) && (rhs == T);
    if ((break_state == break_state::unknown) && (GB6 || GB7 || GB8)) {
        break_state = break_state::dont_break;
    }

    ttlet GB9 = ((rhs == Extend) || (rhs == ZWJ));
    ttlet GB9a = (rhs == SpacingMark);
    ttlet GB9b = (lhs == Prepend);
    if ((break_state == break_state::unknown) & (GB9 || GB9a || GB9b)) {
        break_state = break_state::dont_break;
    }

    ttlet GB11 = state.in_extended_pictograph && (lhs == ZWJ) && (rhs == Extended_Pictographic);
    if ((break_state == break_state::unknown) && GB11) {
        break_state = break_state::dont_break;
    }

    if (rhs == Extended_Pictographic) {
        state.in_extended_pictograph = true;
    } else if (!((rhs == Extend) || (rhs == ZWJ))) {
        state.in_extended_pictograph = false;
    }

    ttlet GB12_13 = (lhs == Regional_Indicator) && (rhs == Regional_Indicator) && ((state.RI_count % 2) == 1);
    if ((break_state == break_state::unknown) && (GB12_13)) {
        break_state = break_state::dont_break;
    }

    if (rhs == Regional_Indicator) {
        state.RI_count++;
    } else {
        state.RI_count = 0;
    }

    // GB999
    if (break_state == break_state::unknown) {
        break_state = break_state::do_break;
    }

    state.previous = rhs;
    return break_state == break_state::do_break;
}

[[nodiscard]] bool breaks_grapheme(char32_t code_point, grapheme_break_state &state) noexcept
{
    return breaks_grapheme(unicode_description::find(code_point).grapheme_cluster_break(), state);
}

} // namespace tt::inline v1
