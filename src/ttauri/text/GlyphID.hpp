// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/text/Grapheme.hpp"
#include "ttauri/tagged_id.hpp"
#include <algorithm>
#include <utility>

namespace tt {

struct glyph_id_tag {};

using GlyphID = tagged_id<uint16_t, glyph_id_tag>;

};

