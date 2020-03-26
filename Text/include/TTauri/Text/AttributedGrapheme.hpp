// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/Grapheme.hpp"
#include "TTauri/Text/TextStyle.hpp"

namespace TTauri::Text {

struct AttributedGrapheme {
    Grapheme grapheme;
    /** All information about the shape and color needed to render this grapheme. */
    TextStyle style;
    ssize_t index;

    AttributedGrapheme(Grapheme grapheme, TextStyle style, ssize_t index=0) :
        grapheme(std::move(grapheme)), style(std::move(style)), index(index) {}
};

}