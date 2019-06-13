// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <boost/exception/exception.hpp>
#include <filesystem>

namespace TTauri {
struct wsRGBApm;
}

namespace TTauri::Draw {

template<typename T> struct PixelMap;

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
PixelMap<wsRGBApm> loadPNG(const PixelMap<wsRGBApm> &pixelMap, const std::filesystem::path &path);

}