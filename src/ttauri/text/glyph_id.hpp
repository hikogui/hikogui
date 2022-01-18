// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../unicode/grapheme.hpp"
#include "../tagged_id.hpp"
#include <algorithm>
#include <utility>

namespace tt::inline v1 {

using glyph_id = tagged_id<uint16_t, "glyph_id">;

}
