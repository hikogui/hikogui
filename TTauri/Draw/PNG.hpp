// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri {
struct wsRGBA;
class URL;
}

namespace TTauri::Draw {

template<typename T> struct PixelMap;

/*! Load a PNG image into the pixel map.
 * pixelMap is required to be large enough to load the png file into.
 *
 * \return A pixelMap the size of the actual PNG file, a submap() from pixelMap.
 */
PixelMap<wsRGBA> loadPNG(const PixelMap<wsRGBA> &pixelMap, const URL &path);

}
