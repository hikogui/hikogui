// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "unicode_grapheme_cluster_break.hpp"

namespace tt {

struct grapheme_break_state {
    unicode_grapheme_cluster_break previous = unicode_grapheme_cluster_break::Other;
    int RI_count = 0;
    bool first_character = true;
    bool in_extended_pictograph = false;

    void reset() noexcept
    {
        previous = unicode_grapheme_cluster_break::Other;
        RI_count = 0;
        first_character = true;
        in_extended_pictograph = false;
    }
};

/** Check if for a grapheme break before the given code-point.
 * Code points must be tested in order, starting at the beginning of the text.
 *
 * \param code point Current code point to test.
 * \param state Current state of the grapheme-break algorithm.
 * \return true when a grapheme break exists before the current code-point.
 */
[[nodiscard]] bool breaks_grapheme(char32_t code_point, grapheme_break_state &state) noexcept;

}