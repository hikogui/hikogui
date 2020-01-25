// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <unordered_map>
#include <string>

namespace TTauri::Text {

/** Describes how a grapheme should be underlined when rendering the text.
* It is carried with the grapheme and glyphs, so that the text render engine
* can draw the decoration after the text is shaped and in rendering-order
* (left to right) and, this makes it easier to correctly render the decoration
* of multiple glyphs in a single stroke.
*/
enum class TextDecoration {
    None,
    Underline,
    WavyUnderline,
    StrikeThrough,

    max = StrikeThrough
};

inline auto const TextDecoration_from_string_table = std::unordered_map<std::string,TextDecoration>{
    {"none", TextDecoration::None},
    {"underline", TextDecoration::Underline},
    {"wavy-underline", TextDecoration::WavyUnderline},
    {"strike-through", TextDecoration::StrikeThrough},
};

}