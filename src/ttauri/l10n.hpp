// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text/language.hpp"
#include "text/translation.hpp"

namespace tt {

/** A localizable string.
 * Used by gettext to extract all msgids from the program into the .pot file.
 */
class l10n {
public:
    l10n() noexcept : _msg_id() {}
    l10n(l10n const &) noexcept = default;
    l10n(l10n &&) noexcept = default;
    l10n &operator=(l10n const &) noexcept = default;
    l10n &operator=(l10n &&) noexcept = default;

    [[nodiscard]] operator bool() const noexcept
    {
        return !_msg_id.empty();
    }

    l10n(std::string_view msg_id) noexcept : _msg_id(msg_id) {}

    [[nodiscard]] std::string_view
    get_translation(long long n = 0, std::vector<language *> const &languages = language::preferred_languages) const noexcept
    {
        return ::tt::get_translation(_msg_id, n, languages);
    }

private:
    std::string _msg_id;
};

} // namespace tt
