// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "PixelMap.hpp"

#include <filesystem>

namespace TTauri::Draw {

struct PNGError : virtual boost::exception, virtual std::exception {};
struct PNGFileOpenError : virtual PNGError {};
struct PNGReadError : virtual PNGError {};
struct PNGHeaderError : virtual PNGError {};
struct PNGInitializationError : virtual PNGError {};
struct PNGParseError : virtual PNGError {};
struct PNGLibraryError : virtual PNGError {};

/*! Load a PNG image into the pixel map.
 * pixelMap is required to be large enough to load the png file into.
 *
 * \return A pixelMap the size of the actual PNG file, a submap() from pixelMap.
 */
PixelMap<uint32_t> loadPNG(const PixelMap<uint32_t> &pixelMap, const std::filesystem::path &path);

}