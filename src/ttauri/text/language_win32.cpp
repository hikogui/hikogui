// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "language.hpp"
#include "../required.hpp"
#include "../log.hpp"
#include "../registry_win32.hpp"
#include <Windows.h>
#include <Winnls.h>

namespace tt::inline v1 {

/**
 * GetUserPreferredUILanguages() returns at most two of the selected languages in random order
 * and can not be used to retrieve the preferred languages the user has selected.
 *
 * The winrt GlobalizationPreferences::Languages returns all languages in the correct order.
 * However winrt header files are incompatible with c++20 co-routines.
 *
 * Therefor the only option available is to read the language list from the registry.
 */
std::vector<language_tag> language::read_os_preferred_languages() noexcept
{
    try {
        ttlet strings = registry_read_current_user_multi_string("Control Panel\\International\\User Profile", "Languages");

        auto r = std::vector<language_tag>{};
        r.reserve(strings.size());
        for (ttlet &string: strings) {
            r.push_back(language_tag{string});
        }
        return r;

    } catch (...) {
        return {language_tag{"en"}};
    }
}

} // namespace tt::inline v1
