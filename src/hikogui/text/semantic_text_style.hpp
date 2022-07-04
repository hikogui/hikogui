// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../exception.hpp"

namespace hi::inline v1 {

enum class semantic_text_style : unsigned char {
    label,
    small_label,
    warning,
    error,
    help,
    placeholder,
    link,

    _size
};

constexpr std::size_t num_semantic_text_styles = static_cast<std::size_t>(semantic_text_style::_size);

} // namespace hi::inline v1
