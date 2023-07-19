// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "translation.hpp"
#include "po_parser.hpp"
#include "../unicode/module.hpp"
#include "../telemetry/module.hpp"
#include "../macros.hpp"

namespace hi {
inline namespace v1 {

struct translation_key {
    std::string msgid;
    language_tag language;

    [[nodiscard]] std::size_t hash() const noexcept
    {
        return hash_mix(msgid, language);
    }

    [[nodiscard]] constexpr friend bool operator==(translation_key const &, translation_key const &) noexcept = default;
};

}} // namespace hi::inline v1

template<>
struct std::hash<hi::translation_key> {
    [[nodiscard]] std::size_t operator()(hi::translation_key const &rhs) const noexcept
    {
        return rhs.hash();
    }
};

namespace hi {
inline namespace v1 {

std::unordered_map<translation_key, std::vector<std::string>> translations;

[[nodiscard]] std::pair<std::string_view, language_tag>
get_translation(std::string_view msgid, long long n, std::vector<language_tag> const &languages) noexcept
{
    // Update only the language in each iteration.
    auto key = translation_key{std::string{msgid}, language_tag{}};

    for (hilet language : languages) {
        key.language = language;

        hilet i = translations.find(key);
        if (i != translations.cend()) {
            hilet plurality = cardinal_plural(language, n, i->second.size());
            hilet &translation = i->second[plurality];
            if (translation.size() != 0) {
                return {translation, language};
            }
        }
    }
    hi_log_debug("No translation found for '{}'", msgid);
    return {msgid, language_tag{"en-Latn-US"}};
}

void add_translation(std::string_view msgid, language_tag language, std::vector<std::string> const &plural_forms) noexcept
{
    auto key = translation_key{std::string{msgid}, language};
    translations[key] = plural_forms;
}

void add_translation(po_translations const &po_translations, language_tag language) noexcept
{
    for (hilet &translation : po_translations.translations) {
        auto msgid = translation.msgctxt.empty() ? translation.msgid : translation.msgctxt + '|' + translation.msgid;
        add_translation(msgid, language, translation.msgstr);
    }
}

}} // namespace hi::inline v1
