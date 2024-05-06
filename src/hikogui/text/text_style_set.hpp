// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../color/color.hpp"
#include "../utility/utility.hpp"
#include "../font/font.hpp"
#include "../unicode/unicode.hpp"
#include "../container/container.hpp"
#include "../macros.hpp"
#include "text_style.hpp"
#include <ostream>
#include <vector>
#include <algorithm>

hi_export_module(hikogui.text : text_style_set);

hi_export namespace hi { inline namespace v1 {

/** A text-style-set includes styles for displaying text with markup.
 * 
*/
class text_style_set {
public:
    constexpr text_style_set() noexcept = default;
    text_style_set(text_style_set const&) noexcept = default;
    text_style_set(text_style_set&&) noexcept = default;
    text_style_set& operator=(text_style_set const&) noexcept = default;
    text_style_set& operator=(text_style_set&&) noexcept = default;
    [[nodiscard]] friend bool operator==(text_style_set const&, text_style_set const&) noexcept = default;

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return _text_styles.empty();
    }

    [[nodiscard]] text_style get(grapheme_attributes const& attributes) const
    {
        auto r = text_style{};

        for (auto const &[mask, style] : _text_styles) {
            if (matches(mask, attributes)) {
                r.apply(style);
            }
        }

        hi_assert(r.complete());
        return r;
    }

    [[nodiscard]] text_style operator[](grapheme_attributes const& attributes) const
    {
        return get(attributes);
    }

    [[nodiscard]] text_style const& front() const
    {
        hi_assert(not _text_styles.empty());
        return _text_styles.front().second;
    }

    constexpr void clear() noexcept
    {
        _text_styles.clear();
    }

    constexpr void push_back(grapheme_attribute_mask const &mask, text_style const &style)
    {
        _text_styles.emplace_back(mask, style);
    }

private:
    std::vector<std::pair<grapheme_attribute_mask, text_style>> _text_styles;
};

}}
