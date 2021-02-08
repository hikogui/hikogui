// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include <unordered_map>
#include <string>
#include <ostream>

namespace tt {

/** Describes how a grapheme should be underlined when rendering the text.
* It is carried with the grapheme and glyphs, so that the text render engine
* can draw the decoration after the text is shaped and in rendering-order
* (left to right) and, this makes it easier to correctly render the decoration
* of multiple glyphs in a single stroke.
*/
enum class text_decoration {
    None,
    Underline,
    WavyUnderline,
    StrikeThrough,

    max = StrikeThrough
};

inline ttlet text_decoration_from_string_table = std::unordered_map<std::string,text_decoration>{
    {"none", text_decoration::None},
    {"underline", text_decoration::Underline},
    {"wavy-underline", text_decoration::WavyUnderline},
    {"strike-through", text_decoration::StrikeThrough},
};

[[nodiscard]] inline char const *to_const_string(text_decoration const &rhs) noexcept
{
    switch (rhs) {
    case text_decoration::None: return "none"; 
    case text_decoration::Underline: return "underline"; 
    case text_decoration::WavyUnderline: return "wavy-underline"; 
    case text_decoration::StrikeThrough: return "strike-through";
    default: tt_no_default();
    }
}

[[nodiscard]] inline std::string to_string(text_decoration const &rhs) noexcept
{
    return to_const_string(rhs);
}

inline std::ostream &operator<<(std::ostream &lhs, text_decoration const &rhs)
{
    return lhs << to_const_string(rhs);
}

}