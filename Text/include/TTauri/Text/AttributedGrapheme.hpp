// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Text/TextStyle.hpp"

namespace TTauri::Text {

struct AttributedGrapheme {
    Grapheme grapheme;
    int index;

    /** All information about the shape and color needed to render this grapheme. */
    TextStyle style;

    AttributedGrapheme(Grapheme grapheme, int index, TextStyle style) :
        grapheme(std::move(grapheme)), index(index), style(std::move(style)) {}
};

}