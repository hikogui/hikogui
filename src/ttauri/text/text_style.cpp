// Copyright 2020 Pokitec
// All rights reserved.

#include "text_style.hpp"
#include "font_book.hpp"
#include "../application.hpp"

namespace tt {

text_style::text_style(std::string_view family_name, tt::font_variant variant, float size, f32x4 color, text_decoration decoration) noexcept :
    text_style(font_book::global->find_family(family_name), variant, size, color, decoration)
{
}

}
