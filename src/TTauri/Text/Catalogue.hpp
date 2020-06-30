// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/expression.hpp"
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

namespace tt {



struct Translation {
    Language &language;

    /** Translation, for each plurality.
     */
    std::vector<std::string> translation;

public:
    Translation(Language &language, std::vector<std::string> translation) :
        language(language), translation(std::move(translation)) {}

    /** Get the translation.
     * @param n The value used for selecting the correct plurality translation.
     */
    [[nodiscard]] std::string const &get(long long n) const noexcept {
        ttlet plurality = std::clamp(language.plurality(n), ssize_t{0}, ssize(translation) - 1);
        return translation[plurality];
    }

};

struct Translations {
    std::vector<Translation> translations;

    /** Get a translation based on the given language order.
     * @param languages The languages to translate to in the prefered order.
     * @param n The value used for selecting the correct plurality translation.
     * @return The translated string, or nullptr if no translation has been found.
     */
    [[nodiscard]] std::string const *get(std::vector<Language *> const &languages, long long n) const noexcept {
        ssize_t best_language = ssize(languages);
        Translation const *best_translation = nullptr;

        for (ttlet &translation: translations) {
            for (ssize_t i = 0; i != best_language; ++i) {
                if (&translation.language == languages[i]) {
                    best_language = i;
                    best_translation = &translation;
                }
            }
        }
        if (best_translation) {
            return &best_translation->get(n);
        } else {
            return nullptr;
        }
    }
};

/** A catalogue of messages.
 */
class TranslationCatalogue {
    std::vector<std::unique_ptr<Language>> languages;

    std::vector<Language *> prefered_language_ptrs;

    // std::map can be searched using a string_view.
    std::unordered_map<std::string,Translations> translation_by_message;

    size_t language_list_cbid;

public:
    TranslationCatalogue() noexcept;

    void set_prefered_languages(std::vector<std::string> const &new_language_list) noexcept;

    void add_translation(std::string original, Translations translation) noexcept {
        translation_by_message[std::move(original)] = std::move(translation);
    }

    /** Get a message from the catalogue.
     * @param original English message used to search the catalogue. The string may include the context following a '|' character.
     * @param original_plural Engligh plural message, used as fallback together with the msgid. May be an empty string_view.
     * @param n Used for plurality determination. If unused set to 1.
     * @return The translated message, or the english fallback.
     */
    [[nodiscard]] std::string_view get(
        std::string const &original,
        std::string_view original_plural,
        unsigned long long n
    ) const noexcept
    {
        ttlet i = translation_by_message.find(original);
        if (i != translation_by_message.end()) {
            if (auto translated = i->second.get(prefered_language_ptrs, n)) {
                return *translated;
            }
        }

        LOG_WARNING("TranslationCatalogue: Missing translation for msgid '{}'", original);
        if (n != 1 && ssize(original_plural) != 0) {
            return original_plural;
        } else {
            return original;
        }
    }
};

TranslationCatalogue parseCatalogue(URL const &url);

}
