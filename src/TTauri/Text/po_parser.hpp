

#pragma once

namespace tt {

struct po_translation_t {
    std::string msgctxt;
    std::string msgid;
    std::string msgid_plural;
    std::vector<std::string> msgstr;
};

struct po_translations_t {
    std::string language;
    int nr_plural_forms;
    std::string plural_expression;
    std::vector<po_translation_t> translations;
};

[[nodiscard]] po_translations_t parse_po(URL const &url);

}

