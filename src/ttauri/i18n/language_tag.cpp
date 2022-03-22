// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// https://www.boost.org/LICENSE_1_0.txt)

#include "language_tag.hpp"
#include "expand_language_tag.hpp"
#include "../check.hpp"
#include <algorithm>

namespace tt::inline v1 {

/** Basic language tag parser.
 *
 * This parser simply translates the given string into a language-tag. It does
 * this without looking at expansion tables.
 */
[[nodiscard]] static language_tag parse_language_tag(std::string_view str)
{
    auto language = iso_639{};
    auto script = iso_15924{};
    auto region = iso_3166{};

    char extension_first_char = 0;

    // Replace underscores to dashes, since invalid language-tags do exist in the real world.
    auto str_ = std::string{str};
    for (auto& c : str_) {
        if (c == '_') {
            c = '-';
        }
    }
    str = std::string_view{str_};

    for (ttlet element : std::views::split(str, std::string_view{"-"})) {
        if (extension_first_char) {
            // Once inside the extensions portion of a language tag you can no
            // longer determine validity based on just the element size.
            ;

        } else if (not language) {
            tt_parse_check(
                (element.size() == 2 or element.size() == 3) and is_alpha(element),
                "First element of a language tag must be a ISO-639 2 or 3 letter language code, got '{}'", str);
            // 2 or 3 letter non-optional ISO-639 language code.
            language = {element};

        } else {
            if (not script and not region and element.size() == 3 and is_alpha(element)) {
                // Up to 3 optional 3 letter extended language codes.
                // Ignore these for backward compatibility.
                ;

            } else if (not script and not region and element.size() == 4 and is_alpha(element)) {
                // The language code may be followed by a 4 letter script code.
                script = {element};

            } else if (
                not region and (element.size() == 2 and is_alpha(element)) or (element.size() == 3 and is_digit(element))) {
                // The language code or script code may also be followed by a 2 letter or 3 digit country code.
                region = {element};

            } else if ((element.size() >= 5 and element.size() <= 8) or (element.size() == 4 and is_digit(element.front()))) {
                // A variant has 5 to 8 letters or a 4 digit + letters code.
                ;

            } else if (element.size() == 1) {
                // Start of an extension. We do not differentiate with private-use indicator.
                extension_first_char = element.front();

            } else {
                throw parse_error(
                    std::format("Unexpected element '{}' while parsing language tag '{}'", std::string_view{element}, str));
            }
        }
    }

    return language_tag{language, script, region};
}

[[nodiscard]] language_tag language_tag::expand() const noexcept
{
    auto r = *this;

    if (script and region) {
        return r;
    }

    if (auto from_language = expand_language_tag(r.language.code())) {
        auto from_language_tag = parse_language_tag(*from_language);

        if (not r.script and from_language_tag.script) {
            r.script = from_language_tag.script;
        }
        if (not r.region and from_language_tag.region) {
            r.region = from_language_tag.region;
        }
    }

    if (script and region) {
        return r;
    }

    if (auto from_region = expand_language_tag(std::string{"und-"} + std::string{r.region.code2()})) {
        auto from_region_tag = parse_language_tag(*from_region);

        if (not r.script and from_region_tag.script) {
            r.script = from_region_tag.script;
        }
    }

    return r;
}

language_tag::language_tag(std::string_view str) : language(), script(), region()
{
    // First do an initial pass over the expansion table to convert likely languages.
    // For example "nl" -> "nl-Latn-NL", "nl-BE" -> "nl-BE".
    if (auto expanded_str = expand_language_tag(str)) {
        str = *expanded_str;
    }

    *this = parse_language_tag(str).expand();
}

[[nodiscard]] std::vector<language_tag> variants(std::vector<language_tag> languages)
{
    auto tmp = std::vector<std::vector<language_tag>>{};

    for (ttlet& language : languages) {
        auto& lang_tmp = tmp.emplace_back();
        for (ttlet& variant : language.all_variants()) {
            lang_tmp.push_back(variant);
        }
    }

    for (auto it = tmp.rbegin(); it != tmp.rend(); ++it) {
        // Remove duplicates in previous language-variant lists.
        for (auto jt = it + 1; jt != tmp.rend(); ++jt) {
            for (ttlet& tag : *it) {
                std::erase(*jt, tag);
            }
        }
    }

    auto r = std::vector<language_tag>{};

    ttlet count = std::accumulate(tmp.begin(), tmp.end(), 0_uz, [](ttlet& value, ttlet& item) {
        return value + item.size();
    });
    r.reserve(count);

    for (ttlet& variants : tmp) {
        for (ttlet& tag : variants) {
            r.push_back(tag);
        }
    }

    return r;
}

} // namespace tt::inline v1