// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../i18n/i18n.hpp"
#include <optional>
#include <string>
#include <vector>
#include <cstddef>

hi_export_module(hikogui.l10n.po_translations)

hi_export namespace hi { inline namespace v1 {

struct po_translation {
    std::optional<std::string> msgctxt;
    std::string msgid;
    std::string msgid_plural;
    std::vector<std::string> msgstr;
};

struct po_translations {
    language_tag language;
    size_t nr_plural_forms;
    std::string plural_expression;
    std::vector<po_translation> translations;
};

}}