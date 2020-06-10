// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Text/TextStyle.hpp"
#include "TTauri/Text/globals.hpp"

namespace TTauri {

TextStyle::TextStyle(std::string_view family_name, TTauri::FontVariant variant, float size, vec color, TextDecoration decoration) noexcept :
    TextStyle(fontBook->find_family(family_name), variant, size, color, decoration) {}

}
