// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_style.hpp"
#include "font_book.hpp"
#include "../application.hpp"

namespace tt {

text_style::text_style(
    std::string_view family_name,
    tt::font_variant variant,
    float size,
    tt::color color,
    text_decoration decoration) noexcept :
    text_style(font_book::global().find_family(family_name), variant, size, color, decoration)
{
}

}
