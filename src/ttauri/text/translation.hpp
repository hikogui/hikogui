// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "language.hpp"
#include "../formula/formula.hpp"
#include "../hash.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace tt {

[[nodiscard]] std::u8string_view get_translation(
    std::u8string_view msgid,
    long long n=0,
    std::vector<language*> const &languages=language::preferred_languages
) noexcept;

void add_translation(
    std::u8string_view msgid,
    language const &language,
    std::vector<std::u8string> const &plural_forms
) noexcept;

void add_translation(
    std::u8string_view msgid,
    std::u8string const &language_tag,
    std::vector<std::u8string> const &plural_forms
) noexcept;

struct po_translations;
void add_translation(po_translations const &translations, language const &language) noexcept;

}
