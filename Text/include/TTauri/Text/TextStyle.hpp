// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Text/FontDescription.hpp"
#include "TTauri/Text/TextDecoration.hpp"
#include "TTauri/Text/FontFamilyID.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"

namespace TTauri::Text {

struct TextStyle {
    FontFamilyID family_id;
    FontVariant variant;
    float size;
    wsRGBA color;
    TextDecoration decoration;

    TextStyle() :
        family_id(), variant(), size(0.0), color(), decoration(TextDecoration::None) {}

    TextStyle(TTauri::Text::FontFamilyID family_id, TTauri::Text::FontVariant variant, float size, wsRGBA color, TextDecoration decoration) :
        family_id(family_id), variant(variant), size(size), color(color), decoration(decoration) {}

    TextStyle(std::string_view family_name, TTauri::Text::FontVariant variant, float size, wsRGBA color, TextDecoration decoration);
};

}