// Copyright 2020 Pokitec
// All rights reserved.

#include "TextStyle.hpp"
#include "font_book.hpp"
#include "../application.hpp"

namespace tt {

TextStyle::TextStyle(std::string_view family_name, tt::FontVariant variant, float size, f32x4 color, TextDecoration decoration) noexcept :
    TextStyle(font_book::global->find_family(family_name), variant, size, color, decoration)
{
}

}
