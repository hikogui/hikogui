// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/tagged_id.hpp"

namespace TTauri::Text {

/** FontID is 15 bits so that it can fit inside the FindIdGlyphs.
*/
using FontID = tagged_id<uint16_t, "font_id"_tag, 0x7ffe>;

}