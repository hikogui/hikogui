// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <cstdint>
#include <compare>

export module hikogui_font_font_family_id;
import hikogui_utility;

export namespace hi::inline v1 {

export using font_family_id = tagged_id<uint16_t, "font_family_id">;

}
