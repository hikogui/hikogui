// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Text/TextStyle.hpp"

namespace TTauri::Text {

struct AttributedGrapheme {
    Grapheme grapheme;

    /** All information about the shape and color needed to render this grapheme.
     */
    TextStyle style;

    /** Index of the grapheme before bidi-algorithm.
     */
    ssize_t logicalIndex;

    AttributedGrapheme(Grapheme grapheme, TextStyle style, ssize_t logicalIndex=0) :
        grapheme(std::move(grapheme)), style(std::move(style)), logicalIndex(logicalIndex) {}
};

}