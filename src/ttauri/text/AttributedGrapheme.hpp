// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Grapheme.hpp"
#include "TextStyle.hpp"
#include "UnicodeData.hpp"

namespace tt {

struct AttributedGrapheme {
    Grapheme grapheme;

    /** All information about the shape and color needed to render this grapheme.
     */
    TextStyle style;

    /** Index of the grapheme before bidi-algorithm.
     */
    ssize_t logicalIndex;

    /** The bidirectional classification.
     */
    BidiClass bidiClass;

    /** Embedding level needed by the bidi-algorithm.
     */
    int8_t embeddingLevel;

    GeneralCharacterClass charClass;

    AttributedGrapheme(Grapheme grapheme, TextStyle style, ssize_t logicalIndex=0) :
        grapheme(std::move(grapheme)), style(std::move(style)), logicalIndex(logicalIndex),
        bidiClass(BidiClass::Unknown), charClass(GeneralCharacterClass::Unknown) {}
};

}