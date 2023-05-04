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

namespace hi { inline namespace v1 {

class text_theme : public std::vector<text_style> {
public:
    using std::vector<text_style>::vector;

    [[nodiscard]] text_style const& find(text_phrasing phrasing, language_tag language) const noexcept
    {
        for (hilet& style : *this) {
            if (matches(style, phrasing, language)) {
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
     * @param language_mask A mask of the language.
     */
    [[nodiscard]] text_style& find_or_add(text_phrasing_mask phrasing_mask, language_tag language_mask) noexcept
    {
        for (auto& style : *this) {
            if (style.phrasing_mask == phrasing_mask and style.language_mask == language_mask) {
                return style;
            }
        }

        // Styles with filters are added before, so that the last style in the
        // CSS file is found first. And the style without filters is the last one.
        auto it = [&]{
            if (empty()) {
                return insert(cbegin(), text_style{});
            } else {
                auto non_filter_style = back();
                return insert(cbegin(), std::move(non_filter_style));
            }
        }();
        
        it->phrasing_mask = phrasing_mask;
        it->language_mask = language_mask;
        return *it;
    }
};

}} // namespace hi::v1

template<typename CharT>
struct std::formatter<hi::text_theme, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text_theme const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(std::format("#{}", t.size()), fc);
    }
};
