// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontDescription.hpp"
#include "TTauri/Text/TextDecoration.hpp"
#include "TTauri/Text/FontFamilyID.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"

namespace TTauri::Text {

struct TextStyle {
    FontFamilyID family_id;
    FontVariant variant;
    float size;
    R16G16B16A16SFloat color;
    float shadow_size;
    TextDecoration decoration;

    TextStyle() :
        family_id(), variant(), size(0.0), color(), shadow_size(0.0), decoration(TextDecoration::None) {}

    TextStyle(TTauri::Text::FontFamilyID family_id, TTauri::Text::FontVariant variant, float size, R16G16B16A16SFloat color, float shadow_size, TextDecoration decoration) :
        family_id(family_id), variant(variant), size(size), color(color), shadow_size(shadow_size), decoration(decoration) {}

    TextStyle(std::string_view family_name, TTauri::Text::FontVariant variant, float size, R16G16B16A16SFloat color, float shadow_size, TextDecoration decoration);
};

}