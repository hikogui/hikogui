// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "text_style.hpp"
#include "../font/font.hpp"
#include "../macros.hpp"

namespace hi::inline v1 {

[[nodiscard]] float text_sub_style::cap_height() const noexcept
{
    hilet& font = find_font(family_id, variant);
    return font.metrics.cap_height * size;
}

[[nodiscard]] float text_sub_style::x_height() const noexcept
{
    hilet& font = find_font(family_id, variant);
    return font.metrics.x_height * size;
}

} // namespace hi::inline v1
