// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../tagged_id.hpp"

namespace tt {

struct font_id_tag {};

/** font_id is 15 bits so that it can fit inside the FindIdGlyphs.
*/
using font_id = tagged_id<uint16_t, font_id_tag, 0x7ffe>;

}