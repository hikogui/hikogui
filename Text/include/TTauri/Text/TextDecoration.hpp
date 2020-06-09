// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <unordered_map>
#include <string>
#include <ostream>

namespace TTauri {

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

[[nodiscard]] inline char const *to_const_string(TextDecoration const &rhs) noexcept
{
    switch (rhs) {
    case TextDecoration::None: return "none"; 
    case TextDecoration::Underline: return "underline"; 
    case TextDecoration::WavyUnderline: return "wavy-underline"; 
    case TextDecoration::StrikeThrough: return "strike-through";
    default: no_default;
    }
}

[[nodiscard]] inline std::string to_string(TextDecoration const &rhs) noexcept
{
    return to_const_string(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, TextDecoration const &rhs)
{
    return lhs << to_const_string(rhs);
}

}