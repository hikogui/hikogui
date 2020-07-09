// Copyright 2020 Pokitec
// All rights reserved.

#include "TextStyle.hpp"
#include "FontBook.hpp"
#include "../Application.hpp"

namespace tt {

TextStyle::TextStyle(std::string_view family_name, tt::FontVariant variant, float size, vec color, TextDecoration decoration) noexcept :
    TextStyle(application->fonts->find_family(family_name), variant, size, color, decoration) {}

}
