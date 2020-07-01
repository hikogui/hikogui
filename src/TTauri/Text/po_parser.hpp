

#pragma once

namespace tt {

struct po_translation {
    std::string msgctxt;
    std::string msgid;
    std::string msgid_plural;
    std::vector<std::string> msgstr;
};

struct po_translations {
    std::string language;
    int nr_plural_forms;
    std::string plural_expression;
    std::vector<po_translation> translations;
};

[[nodiscard]] po_translations parse_po(URL const &url);

}

