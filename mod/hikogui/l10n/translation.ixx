// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <tuple>

export module hikogui_l10n_translation;
import hikogui_formula;
import hikogui_i18n;
import hikogui_l10n_po_parser;
import hikogui_l10n_po_translations;
import hikogui_settings;
import hikogui_telemetry;
import hikogui_unicode;
import hikogui_utility;

export namespace hi {
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

export namespace hi {
inline namespace v1 {

std::unordered_map<translation_key, std::vector<std::string>> translations;
std::atomic<bool> translations_loaded = false;

void add_translation(std::string_view msgid, language_tag language, std::vector<std::string> const &plural_forms) noexcept
{
    auto key = translation_key{std::string{msgid}, language};
    translations[key] = plural_forms;
}

void add_translations(po_translations const &po_translations) noexcept
{
    for (hilet &translation : po_translations.translations) {
        auto msgid = translation.msgctxt ? *translation.msgctxt + '|' + translation.msgid : translation.msgid;
        add_translation(std::move(msgid), po_translations.language, translation.msgstr);
    }
}

void load_translations(std::filesystem::path path)
{
    hi_log_info("Loading translation file {}.", path.string());
    return add_translations(parse_po(path));
}

void load_translations()
{
    if (not translations_loaded.exchange(true)) {
        // XXX Waiting for C++23 to extend life-time of temporaries in for loops.
        auto resource_paths = resource_dirs();
        for (auto &path : glob(resource_paths, "**/*.po")) {
            try {
                load_translations(path);
            } catch (std::exception const &e) {
                hi_log_error("Could not load translation file. {}", e.what());
            }
        }
    }
}

[[nodiscard]] std::pair<std::string_view, language_tag>
get_translation(std::string_view msgid, long long n, std::vector<language_tag> const &languages) noexcept
{
    load_translations();

    // Update only the language in each iteration.
    auto key = translation_key{std::string{msgid}, language_tag{}};

    for (hilet language : languages) {
        key.language = language;

        hilet i = translations.find(key);
        if (i != translations.cend()) {
            hilet plurality = cardinal_plural(language, n, i->second.size());
            hilet& translation = i->second[plurality];
            if (translation.size() != 0) {
                return {translation, language};
            }
        }
    }
    hi_log_debug("No translation found for '{}'", msgid);
    return {msgid, language_tag{"en-Latn-US"}};
}

}} // namespace hi::inline v1
