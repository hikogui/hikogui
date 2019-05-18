// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Glyph.hpp"
#include <vector>
#include <map>
#include <gsl/gsl>

namespace TTauri::Draw {

struct Font {
    std::map<char32_t,size_t> characterMap;
    std::vector<Glyph> glyphs;
};


}