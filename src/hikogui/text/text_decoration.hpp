// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <unordered_map>
#include <string>
#include <ostream>

hi_export_module(hikogui.text.text_decoration);


hi_export namespace hi::inline v1 {

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
    StrikeThrough
};

// clang-format off
constexpr auto text_decoration_metadata = enum_metadata{
    text_decoration::None, "none",
    text_decoration::Underline, "underline",
    text_decoration::WavyUnderline, "wavy-underline",
    text_decoration::StrikeThrough, "strike-through"
};
// clang-format on

[[nodiscard]] hi_inline std::string_view to_string(text_decoration const &rhs) noexcept
{
    return text_decoration_metadata[rhs];
}

hi_inline std::ostream &operator<<(std::ostream &lhs, text_decoration const &rhs)
{
    return lhs << text_decoration_metadata[rhs];
}

} // namespace hi::inline v1
