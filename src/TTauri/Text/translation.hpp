// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include "language.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace tt {

class translation_type {
    std::vector<std::string> plural_forms;

public:
    language_type const &language;

    translation_type(language_type const &language, std::vector<std::string> plural_forms) noexcept :
        language(language), plural_forms(std::move(plural_forms)) {}

    /** Get the translation.
     * @param n The value used for selecting the correct plurality translation.
     */
    [[nodiscard]] std::string_view get(long long n) const noexcept {
        ttlet plurality = language.plurality(n, ssize(plural_forms));
        return plural_forms[plurality];
    }
};

class translations_type {
    std::vector<translation_type> intrinsic;

public:
    translations_type() noexcept {}

    /** Add a translation.
    * @param language The language for the translation.
    * @param plural_forms The translated text in each plural form.
    */
    void add(language_type const &language, std::vector<std::string> const &plurality_forms) noexcept {
        ttlet i = std::find_if(intrinsic.cbegin(), intrinsic.cend(), [&language](auto translation) {
            return &translation.language == &language;
        });

        if (i == intrinsic.cend()) {
            intrinsic.emplace_back(language, plurality_forms);
        }
    }

    /** Add a translation.
    * @param language_code The language code for the translation. The translation is also
    *                      added for the short-form language code. i.e. 'en' (short) 'en-US' (long).
    * @param plural_forms The translated text in each plural form.
    */
    void add(std::string language_code, std::vector<std::string> const &plurality_forms) noexcept {
        ttlet &language = language::find_or_create(language_code);
        add(language, plurality_forms);

        ttlet short_language_code = split(language_code, "-").front();
        ttlet &short_language = language::find_or_create(short_language_code);
        add(short_language, plurality_forms);
    }

    /** Get a translation based on the given language order.
     * @param languages The languages to translate to in the prefered order.
     * @param n The value used for selecting the correct plurality translation.
     * @return The translated string, or nullptr if no translation has been found.
     */
    [[nodiscard]] std::string_view get(long long n, std::vector<language_type *> const &languages) const noexcept {
        ssize_t best_language = ssize(languages);
        translation const *best_translation = nullptr;

        for (ttlet &translation: intrinsic) {
            for (ssize_t i = 0; i != best_language; ++i) {
                if (&translation.language == languages[i]) {
                    best_language = i;
                    best_translation = &translation;
                }
            }
        }
        if (best_translation) {
            return best_translation->get(n);
        } else {
            return {};
        }
    }
};

/** A catalogue of messages.
 */
class translations_catalogue_type {
    std::unordered_map<std::string,translations_type> translation_by_message;

public:
    translations_catalogue_type() noexcept;

    ~translations_catalogue_type();

    /** Add a translation for a message.
     * @param msgid A string used as an id in the translation catalogue,
     *              most often the text in the native language of the develper.
     * @param language_code The language code for the translation. The translation is also
     *                      added for the short-form language code. i.e. 'en' (short) 'en-US' (long).
     * @param plural_forms The translated text in each plural form.
     */
    void add(
        std::string const &msgid,
        std::string const &language_code,
        std::vector<std::string> const &plural_forms
    ) noexcept {
        auto &translations = translation_by_message[msgid];
        translations.add(language_code, plural_forms);
    }

    /** Get a message from the catalogue.
     * @param msgid A string used as an id in the translation catalogue,
     *              most often the text in the native language of the develper.
     * @param n Used for plurality determination. If unused set this value to 1.
     * @param languages A list of languages to search for translations.
     * @return The translated message, or the msgid as fallback.
     */
    [[nodiscard]] std::string_view get(
        char const *msgid,
        unsigned long long n,
        std::vector<language_type *> languages
    ) const noexcept {
        ttlet i = translation_by_message.find(msgid);
        if (i != translation_by_message.end()) {
            auto translated = i->second.get(n, languages);
            if (ssize(translated) != 0) {
                return translated;
            }
        }

        LOG_WARNING("Translation catalogue: Missing translation for msgid '{}'", msgid);
        return msgid;
    }
};

inline translations_catalogue_type translations;

}
