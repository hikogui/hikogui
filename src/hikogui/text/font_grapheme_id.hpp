// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../unicode/grapheme.hpp"
#include "../hash.hpp"

namespace hi::inline v1 {

/** Combined font_id + grapheme for use as a key in a std::unordered_map.
 */
struct font_grapheme_id {
    hi::font const *font;
    grapheme g;

    font_grapheme_id(hi::font const &font, grapheme g) noexcept : font(&font), g(std::move(g)) {}

    [[nodiscard]] std::size_t hash() const noexcept
    {
        hi_assert_not_null(font);
        return hash_mix(reinterpret_cast<ptrdiff_t>(font), g);
    }

    [[nodiscard]] friend bool operator==(font_grapheme_id const &lhs, font_grapheme_id const &rhs) noexcept
    {
        hi_assert_not_null(lhs.font);
        hi_assert_not_null(rhs.font);
        return (lhs.font == rhs.font) and (lhs.g == rhs.g);
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::font_grapheme_id> {
    [[nodiscard]] std::size_t operator()(hi::font_grapheme_id const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
