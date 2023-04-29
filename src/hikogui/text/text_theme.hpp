// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_style.hpp"
#include "../concurrency/module.hpp"
#include <vector>
#include <array>
#include <algorithm>
#include <cstdint>

namespace hi::inline v1 {

class text_theme : public std::vector<text_style> {
public:
    using std::vector<text_style>::vector;

    [[nodiscard]] text_style const&
    find(text_phrasing phrasing, iso_639 language, iso_3166 region, iso_15924 script) const noexcept
    {
        for (hilet& style : *this) {
            if (matches(style, phrasing, language, region, script)) {
                return style;
            }
        }
        hi_axiom(not empty());
        return back();
    }

    /** Find or add an text_style to the text-theme.
     *
     * This function is used by the style_sheet::activate() to populate
     * the text theme. Therefor this function look for an exact match
     * for the phrasing and language, if not found it will create a new entry.
     *
     * @param phrasing_mask A mask of phrasing values.
     * @param language The language set for this style, or empty for wildcard.
     * @param region The region set for this style, or empty for wildcard.
     * @param script The script set for this style, or empty for wildcard.
     */
    [[nodiscard]] text_style&
    find_or_add(text_phrasing_mask phrasing_mask, iso_639 language, iso_3166 region, iso_15924 script) noexcept
    {
        for (auto& style : *this) {
            if (style.phrasing_mask == phrasing_mask and style.language == language and style.region == region and
                style.script == script) {
                return style;
            }
        }
        auto& style = *this.emplace_back();
        style.phrasing_mask = phrasing_mask;
        style.language = language;
        style.region = region;
        style.script = script;
        return style;
    };

    /** Sort the list of text-styles.
     *
     * Sort the styles from specific to generic, so that find can do a simple
     * forward search for the best match.
     */
    void sort() noexcept
    {
        hi_not_implemented();
    }

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::text_theme, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text_theme const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(std::format("#{}", t.size()), fc);
    }
};
