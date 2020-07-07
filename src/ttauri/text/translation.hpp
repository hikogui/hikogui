// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/foundation/expression.hpp"
#include "ttauri/foundation/hash.hpp"
#include "language.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace tt {

[[nodiscard]] std::string_view get_translation(
    std::string_view msgid,
    long long n=0,
    std::vector<language*> const &languages=*language::preferred_languages
) noexcept;

void add_translation(
    std::string_view msgid,
    language const &language,
    std::vector<std::string> const &plural_forms
) noexcept;

void add_translation(
    std::string_view msgid,
    std::string const &language_tag,
    std::vector<std::string> const &plural_forms
) noexcept;

struct po_translations;
void add_translation(po_translations const &translations, language const &language) noexcept;

}
