// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_general_category.hpp"
#include "unicode_description.hpp"

namespace tt::inline v1 {

[[nodiscard]] bool is_general_category_C(char32_t rhs) noexcept
{
    return is_C(unicode_description::find(rhs).general_category());
}

}
