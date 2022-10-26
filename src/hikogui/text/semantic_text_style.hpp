// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../enum_metadata.hpp"

namespace hi::inline v1 {

enum class semantic_text_style : unsigned char { label, small_label, warning, error, help, placeholder, link };

// clang-format off
constexpr auto semantic_text_style_metadata = enum_metadata{
    semantic_text_style::label, "label",
    semantic_text_style::small_label, "small-label",
    semantic_text_style::warning, "warning",
    semantic_text_style::error, "error",
    semantic_text_style::help, "help",
    semantic_text_style::placeholder, "placeholder",
    semantic_text_style::link, "link",
};
// clang-format on

} // namespace hi::inline v1
