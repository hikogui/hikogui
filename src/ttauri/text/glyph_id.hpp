// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "grapheme.hpp"
#include "../tagged_id.hpp"
#include <algorithm>
#include <utility>

namespace tt {

struct glyph_id_tag {};

using glyph_id = tagged_id<uint16_t, glyph_id_tag>;

};

