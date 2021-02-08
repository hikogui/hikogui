// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "../tagged_id.hpp"

namespace tt {

struct font_id_tag {};

/** font_id is 15 bits so that it can fit inside the FindIdGlyphs.
*/
using font_id = tagged_id<uint16_t, font_id_tag, 0x7ffe>;

}