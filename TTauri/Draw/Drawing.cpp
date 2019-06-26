// Copyright 2019 Pokitec
// All rights reserved.

#include "Drawing.hpp"

namespace TTauri::Draw {

void Drawing::addPath(Path const &path, wsRGBApm const &color)
{
    layers.emplace_back(path, color);
}

void Drawing::addStroke(Path const &path, wsRGBApm const &color, float strokeWidth, LineJoinStyle lineJoinStyle)
{
    auto strokePath = Path{};

    strokePath.addPathToStroke(path, strokeWidth, lineJoinStyle);
    addPath(strokePath, color);
}

void draw(PixelMap<wsRGBApm> &dst, Drawing const &src, SubpixelOrientation subpixelOrientation)
{
    for (let &[path, color]: src.layers) {
        fill(dst, color, path, subpixelOrientation);
    }
}

Drawing operator*(glm::mat3x3 const &lhs, Drawing const &rhs)
{
    auto r = Drawing{};
    r.layers.reserve(rhs.layers.size());

    for (let &[path, color]: rhs.layers) {
        r.layers.emplace_back(lhs * path, color);
    }
    return r;
}

Drawing &operator*=(Drawing &lhs, glm::mat3x3 const &rhs)
{
    for (auto &[path, color]: lhs.layers) {
        path *= rhs;
    }
    return lhs;
}

}