// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <limits>
#include "TTauri/Foundation/required.hpp"

namespace TTauri::GUI::PipelineMSDF {

/* A location inside the atlas where the character is located.
 */
struct Page {
    uint16_t x;
    uint16_t y;
    uint8_t z;
    uint8_t width;
    uint8_t height;
};

}
