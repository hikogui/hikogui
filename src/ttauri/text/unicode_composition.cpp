// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "unicode_composition.hpp"
#include "ttauri/text/unicode_db.hpp"

namespace tt {

[[nodiscard]] char32_t unicode_composition_find(char32_t first, char32_t second) noexcept
{
    auto first_it = std::begin(detail::unicode_db_composition_table);
    auto last_it = std::end(detail::unicode_db_composition_table);

    auto it = unicode_composition_find(first_it, last_it, first, second);
    if (it == last_it) {
        return U'\uffff';
    } else {
        return it->composed();
    }
}

}
