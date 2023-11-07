// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <compare>

hi_export_module(hikogui.font.font_family_id);

hi_export namespace hi::inline v1 {

hi_export using font_family_id = tagged_id<uint16_t, "font_family_id">;

}
