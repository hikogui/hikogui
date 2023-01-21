// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "iso_15924.hpp"
#include "iso_3166.hpp"
#include "iso_639.hpp"
#include "../utility/module.hpp"
#include "../generator.hpp"
#include "../ranges.hpp"
#include <vector>
#include <string_view>

namespace hi::inline v1 {

/** The IETF BCP 47 language tag.
 *
 * This class stores the language tag in 64 bits; in its individual components of the:
 * ISO-639 language (16 bit), ISO-15924 script (16 bit) and ISO-3166 region (16 bit).
 * In the future another 16 bits can be used to store the variants and extensions.
 *
 */
class language_tag {
public:
    iso_639 language;
    iso_15924 script;
    iso_3166 region;
    uint16_t _reserved = 0;

    constexpr language_tag() noexcept {}
    constexpr language_tag(language_tag const&) noexcept = default;
    constexpr language_tag(language_tag&&) noexcept = default;
    constexpr language_tag& operator=(language_tag const&) noexcept = default;
    constexpr language_tag& operator=(language_tag&&) noexcept = default;

    constexpr language_tag(iso_639 const& language, iso_15924 const& script = {}, iso_3166 const& region = {}) noexcept :
        language(language), script(script), region(region)
    {
    }

    constexpr language_tag(iso_639 const& language, iso_3166 const& region) noexcept : language_tag(language, {}, region) {}

    /** Parse a language tag.
     *
     * This will construct a language tag with the language, script and region set
     * as complete as possible. It does this by expanding the language tags using
     * default script, region and grandfathering tables.
     *
     * This function will ignore upper/lower case of the sub-tags, and allow for both
     * underscores '_' and dashes '-' to separate the sub-tags.
     *
     * @param str The language tag to parse
     * @throws parse_error
     */
    language_tag(std::string_view str);

    /** Parse the language, script and region raw from the string.
     *
     * No automatic expansion of the script or region will be done.
     */
    [[nodiscard]] static language_tag parse(std::string_view str);

    /** Check if the language tag is empty.
     */
    [[nodiscard]] bool empty() const noexcept
    {
        return language.empty() and script.empty() and region.empty();
    }

    /** Check if the language tag is used.
     */
    explicit operator bool() const noexcept
    {
        return not empty();
    }

    /** Get variants of the language_tag.
     *
     * This function will create language_tags that includes this tag
     * and tags with strictly less information (no script, no region).
     *
     * @return A list of language-tags sorted: lang-script-region, lang-region, lang-script, lang
     */
    [[nodiscard]] generator<language_tag> variants() const noexcept
    {
        co_yield *this;
        if (script and region) {
            co_yield language_tag{language, region};
            co_yield language_tag{language, script};
        }
        if (script or region) {
            co_yield language_tag{language};
        }
    }

    /** Get variants of the language_tag.
     *
     * This function will create language_tags that may include this tag
     * and tags with strictly less information (no script, no region), which still
     * canonically expands into this tag.
     *
     * @return A list of language-tags sorted: lang-script-region, lang-region, lang-script, lang
     */
    [[nodiscard]] generator<language_tag> canonical_variants() const noexcept
    {
        hilet check = expand();
        for (hilet& tag : variants()) {
            if (tag.expand() == check) {
                co_yield tag;
            }
        }
    }

    /** Creates variants of a language tag, including those by expanding the normal variants.
     */
    [[nodiscard]] std::vector<language_tag> all_variants() const noexcept
    {
        auto r = make_vector(variants());

        // And languages variants from expanded variants.
        for (hilet variant : variants()) {
            for (hilet expanded_variant : variant.expand().variants()) {
                if (std::find(r.begin(), r.end(), expanded_variant) == r.end()) {
                    r.push_back(expanded_variant);
                }
            }
        }
        return r;
    }

    /** Expand the language tag to include script and language.
     *
     * Expansion is done by querying default script, default language and grandfathering tables.
     */
    [[nodiscard]] language_tag expand() const noexcept;

    /** Get a tag with only the language.
     */
    [[nodiscard]] language_tag shrink() const noexcept
    {
        auto last_variant = *this;
        for (hilet& variant : canonical_variants()) {
            last_variant = variant;
        }
        return last_variant;
    }

    /** The language direction for this language-tag.
     *
     * @return Either `unicode_bidi_class::L` for left-to-right; or `unicode_bidi_class::R` for right-to-left
     */
    [[nodiscard]] unicode_bidi_class writing_direction() const noexcept
    {
        return expand().script.writing_direction();
    }

    [[nodiscard]] std::string to_string() const noexcept
    {
        auto r = std::string{};
        r += language.code();
        if (script) {
            r += '-';
            r += script.code4();
        }
        if (region) {
            r += "-";
            r += region.code2();
        }
        return r;
    }

    /** Check if two language_tags match for their non-empty fields.
     */
    [[nodiscard]] constexpr friend bool match(language_tag const& lhs, language_tag const& rhs) noexcept
    {
        if (lhs.language != rhs.language) {
            return false;
        }
        if (lhs.script and rhs.script and lhs.script != rhs.script) {
            return false;
        }
        if (lhs.region and rhs.region and lhs.region != rhs.region) {
            return false;
        }
        return true;
    }

    [[nodiscard]] constexpr friend bool operator==(language_tag const&, language_tag const&) noexcept = default;
};

/** Add variants to the list of languages.
 *
 * This function is mostly used to add languages to a list of preferred languages
 * to search for translations in the translation catalog.
 *
 * @param languages A list of languages ordered by preference.
 * @return A new list of languages which includes variants and ordered by the given list of languages.
 */
[[nodiscard]] std::vector<language_tag> variants(std::vector<language_tag> languages);

} // namespace hi::inline v1

template<>
struct std::hash<hi::language_tag> {
    [[nodiscard]] size_t operator()(hi::language_tag const& rhs) const noexcept
    {
        return hi::hash_mix(
            std::hash<hi::iso_639>{}(rhs.language),
            std::hash<hi::iso_15924>{}(rhs.script),
            std::hash<hi::iso_3166>{}(rhs.region));
    }
};

template<typename CharT>
struct std::formatter<hi::language_tag, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::language_tag const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(t.to_string(), fc);
    }
};
