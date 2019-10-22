// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/Font.hpp"
#include "TTauri/Foundation/Path.hpp"
#include "TTauri/Foundation/PathString.hpp"
#include "TTauri/Foundation/wsRGBA.hpp"
#include "TTauri/Foundation/required.hpp"

#include <string>
#include <vector>
#include <memory>

namespace TTauri {

//  
struct Palette {
    /*! Color used by normal text and lines.
     */
    wsRGBA foregroundColor;

    /*! Color used to accent an element which would normally use the foregroundColor.
     */
    wsRGBA accentColor;

    /*! Color used to for a background an element.
     */
    wsRGBA backgroundColor;
};

enum class TextDecoration : uint8_t {
    Normal,
    Underline,
    DoubleUnderline,
    WavyUnderline,
    StrikeThrough
};

struct TextStyle {
    size_t paletteIndex;
    size_t fontIndex;
    TextDecoration decoration; 
};

class theme {
    std::vector<Palette> palettes;
    std::vector<TextStyle> textStyles;
    std::vector<std::pair<Font *,uint8_t>> fontsWithFallBack;
};


}
