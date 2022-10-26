// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../i18n/language_tag.hpp"
#include <string>
#include <vector>
#include <filesystem>

namespace hi::inline v1 {

struct po_translation {
    std::string msgctxt;
    std::string msgid;
    std::string msgid_plural;
    std::vector<std::string> msgstr;
};

struct po_translations {
    language_tag language;
    int nr_plural_forms;
    std::string plural_expression;
    std::vector<po_translation> translations;
};

[[nodiscard]] po_translations parse_po(std::filesystem::path const &path);

} // namespace hi::inline v1
