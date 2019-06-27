// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "Path.hpp"
#include "TTauri/Color.hpp"
#include <utility>

namespace TTauri::Draw {

struct Drawing {
    std::vector<std::pair<Path,wsRGBA>> layers;

    void addPath(Path const &path, wsRGBA const &color);
    void addStroke(Path const &path, wsRGBA const &color, float strokeWidth, LineJoinStyle lineJoinStyle=LineJoinStyle::Miter);
};

void draw(PixelMap<wsRGBA> &dst, Drawing const &src, SubpixelOrientation subpixelOrientation);

Drawing operator*(glm::mat3x3 const &lhs, Drawing const &rhs);
Drawing &operator*=(Drawing &lhs, glm::mat3x3 const &rhs);

}