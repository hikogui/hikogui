// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/vec.hpp"

namespace TTauri::GUI {

enum class SubpixelOrientation {
    Unknown,
    BlueRight,
    BlueLeft,
    BlueBottom,
    BlueTop,
};

/** Return the offset of the blue subpixel compared to the center of the pixel.
 */
inline vec blueOffset(SubpixelOrientation rhs) noexcept {
    constexpr float over3 = 1.0f / 3.0f;
    switch (rhs) {
    case SubpixelOrientation::Unknown:    return { 0.0f,  0.0f};
    case SubpixelOrientation::BlueRight:  return { over3, 0.0f};
    case SubpixelOrientation::BlueLeft:   return {-over3, 0.0f};
    case SubpixelOrientation::BlueBottom: return { 0.0f, -over3};
    case SubpixelOrientation::BlueTop:    return { 0.0f,  over3};
    default: no_default;
    }
}

}