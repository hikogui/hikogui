// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


#include "translation.hpp"
#include "po_parser.hpp"

namespace tt {

struct translation_key {
    std::u8string msgid;
    language const *language;

    translation_key(std::u8string_view msgid, tt::language const *language=nullptr) noexcept :
        msgid(msgid), language(language) {}

    [[nodiscard]] size_t hash() const noexcept {
        return hash_mix(language, msgid);
    }

    [[nodiscard]] friend bool operator==(translation_key const &lhs, translation_key const &rhs) noexcept {
        return lhs.language == rhs.language && lhs.msgid == rhs.msgid;
    }
};

}

namespace std {

template<>
struct hash<tt::translation_key> {
    [[nodiscard]] size_t operator()(tt::translation_key const &rhs) const noexcept {
        return rhs.hash();
    }
};

}

namespace tt {

std::unordered_map<translation_key,std::vector<std::u8string>> translations;

[[nodiscard]] std::u8string_view get_translation(
    std::u8string_view msgid,
    long long n,
    std::vector<language*> const &languages
) noexcept {

    auto key = translation_key{msgid};

    for (ttlet *language : languages) {
        key.language = language;

        ttlet i = translations.find(key);
        if (i != translations.cend()) {
            ttlet plurality = language->plurality(n, std::ssize(i->second));
            ttlet &translation = i->second[plurality];
            if (translation.size() != 0) {
                return translation;
            }
        }
    }
    tt_log_warning("No translation found for '{}'", tt::to_string(msgid));
    return msgid;
}

void add_translation(
    std::u8string_view msgid,
    language const &language,
    std::vector<std::u8string> const &plural_forms
) noexcept {
    auto key = translation_key{msgid, &language};
    translations[key] = plural_forms;
}

void add_translation(
    std::u8string_view msgid,
    language_tag const &language_tag,
    std::vector<std::u8string> const &plural_forms
) noexcept {
    ttlet &language = language::find_or_create(language_tag);
    add_translation(msgid, language, plural_forms);
}

void add_translation(po_translations const &po_translations, language const &language) noexcept
{
    for (ttlet &translation : po_translations.translations) {
        auto msgid = std::ssize(translation.msgctxt) == 0 ? translation.msgid : translation.msgctxt + u8'|' + translation.msgid;
        add_translation(
            msgid,
            language,
            translation.msgstr
        );
    }
}

}
