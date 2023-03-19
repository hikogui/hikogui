// Copyright Take Vos 2020, 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ucd_index.hpp"
#include "ucd_descriptions.hpp"
#include "ucd_compositions.hpp"
#include "unicode_description.hpp"
#include "../utility/module.hpp"

namespace hi::inline v1 {

[[nodiscard]] unicode_description const& unicode_description::find(char32_t code_point) noexcept
{
    return ucd_descriptions[ucd_get_index(code_point)];
}

} // namespace hi::inline v1
