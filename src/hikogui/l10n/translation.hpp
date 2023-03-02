// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../i18n/module.hpp"
#include "../formula/formula.hpp"
#include "../utility/module.hpp"
#include "../os_settings.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <tuple>

namespace hi {
inline namespace v1 {

[[nodiscard]] std::pair<std::string_view, language_tag> get_translation(
    std::string_view msgid,
    long long n = 0,
    std::vector<language_tag> const &languages = os_settings::language_tags()) noexcept;

void add_translation(std::string_view msgid, language_tag language, std::vector<std::string> const &plural_forms) noexcept;

struct po_translations;
void add_translation(po_translations const &translations, language_tag const &language) noexcept;

}} // namespace hi::inline v1
