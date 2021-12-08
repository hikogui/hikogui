// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../exception.hpp"

namespace tt::inline v1 {

enum class theme_text_style : unsigned char {
    label,
    small_label,
    warning,
    error,
    help,
    placeholder,
    link,

    _size
};

constexpr std::size_t num_theme_text_styles = static_cast<std::size_t>(theme_text_style::_size);

} // namespace tt::inline v1
