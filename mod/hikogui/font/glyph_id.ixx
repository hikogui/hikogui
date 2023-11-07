// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <algorithm>
#include <utility>

export module hikogui_font_glyph_id;
import hikogui_unicode;
import hikogui_utility;

export namespace hi::inline v1 {

export using glyph_id = tagged_id<uint16_t, "glyph_id">;

}
