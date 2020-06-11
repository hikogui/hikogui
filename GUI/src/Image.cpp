// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/GUI/Image.hpp"
#include "TTauri/Foundation/png.hpp"

namespace TTauri {

Image::Image(PixelMap<R16G16B16A16SFloat> &&image) noexcept :
    image(std::move(image)) {}

Image::Image(FontGlyphIDs const &image) noexcept :
    image(image) {}

Image::Image(URL const &url) :
    Image(png::load(url))
{

}


}