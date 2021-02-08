// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include "TextStyle.hpp"
#include "unicode_bidi_class.hpp"
#include "unicode_general_category.hpp"

namespace tt {

struct attributed_grapheme {
    Grapheme grapheme;

    /** All information about the shape and color needed to render this grapheme.
     */
    TextStyle style;

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

    attributed_grapheme(Grapheme grapheme, TextStyle style, ssize_t logicalIndex=0) :
        grapheme(std::move(grapheme)), style(std::move(style)), logicalIndex(logicalIndex),
        bidi_class(unicode_bidi_class::unknown),
        general_category(unicode_general_category::unknown)
    {
    }
};

}
