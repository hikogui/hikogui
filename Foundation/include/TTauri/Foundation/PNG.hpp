// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri {
class R16G16B16A16SFloat;
class URL;
}

namespace TTauri {

template<typename T> struct PixelMap;

/*! Load a PNG image into the pixel map.
 * pixelMap is required to be large enough to load the png file into.
 *
 * \return A pixelMap the size of the actual PNG file, a submap() from pixelMap.
 */
PixelMap<R16G16B16A16SFloat> loadPNG(const PixelMap<R16G16B16A16SFloat> &pixelMap, const URL &path);

}
