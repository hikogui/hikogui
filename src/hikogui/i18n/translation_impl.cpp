// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translation.hpp"
#include "po_parser.hpp"
#include "../log.hpp"

namespace hi::inline v1 {

struct translation_key {
    std::string msgid;
    language const *language;

    translation_key(std::string_view msgid, hi::language const *language = nullptr) noexcept : msgid(msgid), language(language) {}

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return hash_mix(language, msgid);
    }

    [[nodiscard]] friend bool operator==(translation_key const &lhs, translation_key const &rhs) noexcept
    {
        return lhs.language == rhs.language && lhs.msgid == rhs.msgid;
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::translation_key> {
    [[nodiscard]] std::size_t operator()(hi::translation_key const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

namespace hi::inline v1 {

std::unordered_map<translation_key, std::vector<std::string>> translations;

[[nodiscard]] std::string_view
get_translation(std::string_view msgid, long long n, std::vector<language *> const &languages) noexcept
{
    auto key = translation_key{msgid};

    for (hilet *language : languages) {
        hi_axiom_not_null(language);

        key.language = language;

        hilet i = translations.find(key);
        if (i != translations.cend()) {
            hilet plurality = language->plurality(n, ssize(i->second));
            hilet &translation = i->second[plurality];
            if (translation.size() != 0) {
                return translation;
            }
        }
    }
    hi_log_debug("No translation found for '{}'", msgid);
    return msgid;
}

void add_translation(std::string_view msgid, language const &language, std::vector<std::string> const &plural_forms) noexcept
{
    auto key = translation_key{msgid, &language};
    translations[key] = plural_forms;
}

void add_translation(
    std::string_view msgid,
    language_tag const &language_tag,
    std::vector<std::string> const &plural_forms) noexcept
{
    hilet &language = language::find_or_create(language_tag);
    add_translation(msgid, language, plural_forms);
}

void add_translation(po_translations const &po_translations, language const &language) noexcept
{
    for (hilet &translation : po_translations.translations) {
        auto msgid = ssize(translation.msgctxt) == 0 ? translation.msgid : translation.msgctxt + '|' + translation.msgid;
        add_translation(msgid, language, translation.msgstr);
    }
}

} // namespace hi::inline v1
