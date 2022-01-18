// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_style.hpp"
#include "../unicode/grapheme.hpp"
#include "../unicode/unicode_bidi_class.hpp"
#include "../unicode/unicode_general_category.hpp"

namespace tt::inline v1 {

struct attributed_grapheme {
    tt::grapheme grapheme;

    /** All information about the shape and color needed to render this grapheme.
     */
    text_style style;

    /** Index of the grapheme before bidi-algorithm.
     */
    ssize_t logicalIndex;

    /** The bidirectional classification.
     */
    unicode_bidi_class bidi_class;

    /** Embedding level needed by the bidi-algorithm.
     */
    int8_t embeddingLevel;

    unicode_general_category general_category;

    attributed_grapheme(tt::grapheme grapheme, text_style style, ssize_t logicalIndex = 0) :
        grapheme(std::move(grapheme)),
        style(std::move(style)),
        logicalIndex(logicalIndex),
        bidi_class(unicode_bidi_class::unknown),
        general_category(unicode_general_category::Cn)
    {
    }
};

} // namespace tt::inline v1
