// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/tagged_id.hpp"

namespace tt {

struct font_id_tag {};

/** FontID is 15 bits so that it can fit inside the FindIdGlyphs.
*/
using FontID = tagged_id<uint16_t, font_id_tag, 0x7ffe>;

}