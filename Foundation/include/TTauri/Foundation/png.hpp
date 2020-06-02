

#pragma once

#include "TTauri/Foundation/PixelMap.hpp"
#include "TTauri/Foundation/R16G16B16A16SFloat.hpp"
#include <nonstd/span>

namespace TTauri {

void png_decode(nonstd::span<std::byte> bytes, PixelMap<R16G16B16A16SFloat> &image);


void png_decode(URL const &url, PixelMap<R16G16B16A16SFloat> &image);

PixelMap<R16G16B16A16SFloat> png_decode(nonstd::span<std::byte> bytes);

PixelMap<R16G16B16A16SFloat> png_decode(URL const &url);

};

