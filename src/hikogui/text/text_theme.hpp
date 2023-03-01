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
    [[nodiscard]] text_style const &
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
};

} // namespace hi::inline v1

template<typename CharT>
struct std::formatter<hi::text_theme, CharT> : std::formatter<std::string_view, CharT> {
    auto format(hi::text_theme const& t, auto& fc)
    {
        return std::formatter<std::string_view, CharT>::format(std::format("#{}", t.size()), fc);
    }
};
