

#pragma once

#include <string>
#include <vector>
#include "language_tag.hpp"
#include "../URL.hpp"

namespace tt {

struct po_translation {
    std::u8string msgctxt;
    std::u8string msgid;
    std::u8string msgid_plural;
    std::vector<std::u8string> msgstr;
};

struct po_translations {
    language_tag language;
    int nr_plural_forms;
    std::u8string plural_expression;
    std::vector<po_translation> translations;
};

[[nodiscard]] po_translations parse_po(URL const &url);

}

