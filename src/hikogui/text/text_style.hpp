// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../i18n/module.hpp"
#include "../font/font_family_id.hpp"
#include "../font/font_variant.hpp"
#include "../color/color.hpp"
#include "text_phrasing.hpp"

namespace hi { inline namespace v1 {

struct text_style {
    text_phrasing_mask phrasing_mask = {};
    language_tag language_mask = {};
    font_family_id family_id = {};
    font_variant variant = {};
    int size = {};
    hi::color color = {};

    constexpr text_style() noexcept = default;
    constexpr text_style(text_style const&) noexcept = default;
    constexpr text_style(text_style&&) noexcept = default;
    constexpr text_style& operator=(text_style const&) noexcept = default;
    constexpr text_style& operator=(text_style&&) noexcept = default;
    [[nodiscard]] constexpr friend bool operator==(text_style const&, text_style const&) noexcept = default;

    constexpr text_style(
        text_phrasing_mask phrasing_mask,
        language_tag language_mask,
        font_family_id family_id,
        font_variant variant,
        int size,
        hi::color color) :
        phrasing_mask(phrasing_mask),
        language_mask(language_mask),
        family_id(family_id),
        variant(variant),
        size(size),
        color(color)
    {
    }

    [[nodiscard]] constexpr friend bool
    matches(text_style const& lhs, text_phrasing phrasing, language_tag language) noexcept
    {
        return matches(lhs.phrasing_mask, phrasing) and matches(lhs.language_mask, language);
    }
};

}} // namespace hi::v1
