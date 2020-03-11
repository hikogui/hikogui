// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/TextStyle.hpp"
#include "TTauri/Text/globals.hpp"

namespace TTauri::Text {

TextStyle::TextStyle(std::string_view family_name, TTauri::Text::FontVariant variant, float size, R16G16B16A16SFloat color, float shadow_size, TextDecoration decoration) :
    TextStyle(Text_globals->font_book->find_family(family_name), variant, size, color, shadow_size, decoration) {}

}