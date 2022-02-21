// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "language.hpp"
#include "../formula/formula.hpp"
#include "../hash.hpp"
#include "../os_settings.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace tt::inline v1 {

[[nodiscard]] std::string_view get_translation(
    std::string_view msgid,
    long long n = 0,
    std::vector<language *> const &languages = os_settings::languages()) noexcept;

void add_translation(std::string_view msgid, language const &language, std::vector<std::string> const &plural_forms) noexcept;

void add_translation(
    std::string_view msgid,
    std::string const &language_tag,
    std::vector<std::string> const &plural_forms) noexcept;

struct po_translations;
void add_translation(po_translations const &translations, language const &language) noexcept;

} // namespace tt::inline v1
