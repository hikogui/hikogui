// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;

#include <optional>
#include <string>
#include <vector>
#include <cstddef>

export module hikogui_l10n_po_translations;
import hikogui_i18n;

export namespace hi { inline namespace v1 {

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