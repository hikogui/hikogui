// Copyright 2019 Pokitec
// All rights reserved.

#include "Glyphs.hpp"

namespace TTauri::Draw {

Glyphs operator*(glm::mat3x3 const &lhs, Glyphs rhs)
{
    return rhs *= lhs;
}

Glyphs &operator*=(Glyphs &lhs, glm::mat3x3 const &rhs)
{
    for (auto &glyph: lhs.glyphs) {
        glyph *= rhs;
    }
    return lhs;
}

}