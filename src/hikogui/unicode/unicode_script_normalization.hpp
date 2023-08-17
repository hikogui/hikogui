

#pragma once

#include "ucd_general_categories.hpp"
#include "ucd_scripts.hpp"

hi_export_module(hikogui.unicode.unicode_script_normalization);

namespace hi { inline namespace v1 {

/** Fixup the language of text.
 *
 * Check the characters in text and make sure the script-attribute does not
 * contradict the Unicode script table. And if the script-attribute for the
 * character was not set, then determine the script.
 *
 * Steps:
 *  1. Replace the script for each grapheme when the base code-point has an
 *     explicit script in the Unicode database.
 *  2. If a language is set for a grapheme, expand the region and script.
 *  3. For each word, including attached punctuation, spread the language
 *     among the graphemes. But leave any explicit script as-is.
 * 
 *  6. For any grapheme with a unset language, region or script use the
 *     default-language-tag.
 *
 * @ingroup unicode
 */
template<typename It, std::sentinel_for<It> ItEnd>
inline void unicode_normalize_script(It first, ItEnd last, language_tag default_language_tag) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, grapheme>)
{
    if (first == last) {
        return;
    }

    // Replace the script for each grapheme when the base code-point has an
    // explicit script in the Unicode database.
    for (auto it = first; it != last; ++it) {
        hilet code_point = (*it)[0];
        hilet udb_script = ucd_get_script(code_point);
        if (udb_script != unicode_script::Zzzz and udb_script != unicode_script::Common) {
            it->script = udb_script;
        }
    }

    // If a language is set for a grapheme, expand the region and script.
    for (auto it = first; it != last; ++it) {
        if (it->language()) {
            it->set_language_tag(it->language_tag().expand());
        }
    }

    // For each word, including attached punctuation, spread the language
    // among the graphemes. But leave any explicit script as-is.
    auto last_language_tag = language_tag{};
    auto word_it = it;
    for (auto it = first; it != last; ++it) {
        auto general_category = ucd_get_general_category((*it)[0]);
        if (is_Z(general_category)) {
            // Found the end of the word.
            word_it = it + 1;
            last_language_tag = {};

        } else if (it->language()) {
            if (not last_language_tag) {
                // Update the language, region and optional script for graphemes
                // earlier in the word.
                for (auto jt = word_it; jt != it; ++jt) {
                    jt->set_language(it->language());
                    jt->set_region(it->region());
                    if (not jt->script()) {
                        jt->set_script(it->script());
                    }
                }
            }
            // This character in the word has a language
            last_language_tag = it->language_tag();
        
        } else (last_language_tag) {
            // Update the language of the current grapheme.
            it->set_language(last_language_tag->language());
            it->set_region(last_language_tag->region());
            if (not it->script()) {
                it->set_script(last_language_tag->script());
            }
        }
    }

    // For any grapheme with a unset language, region or script use the
    // default-language-tag.
    default_language_tag = default_language_tag.expand();
    for (auto it = first; it != last; ++it) {
        if (not it->language()) {
            it->set_language(default_language_tag->language());
            it->set_region(default_language_tag->region());
        }
        if (not it->script()) {
            it->set_script(default_language_tag->script());
        }
    }
}

}} // namespace hi::v1
