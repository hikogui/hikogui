// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../i18n/iso_639.hpp"
#include "../i18n/iso_15924.hpp"
#include "../i18n/iso_3166.hpp"
#include "../font/font_family_id.hpp"
#include "../font/font_variant.hpp"
#include "../color/color.hpp"
#include "text_phrasing.hpp"

namespace hi { inline namespace v1 {

struct text_style {
    text_phrasing_mask phrasing_mask;
    iso_639 language;
    iso_15924 script;
    iso_3166 country;
    font_family_id family_id;
    font_variant variant;
    int size;
    hi::color color;

    constexpr text_style(
        text_phrasing_mask phrasing_mask,
        iso_639 language,
        iso_15924 script,
        iso_3166 country,
        font_family_id family_id,
        font_variant variant,
        int size,
        hi::color color) :
        phrasing_mask(phrasing_mask),
        language(language),
        script(script),
        country(country),
        family_id(family_id),
        variant(variant),
        size(size),
        color(color)
    {
    }
};

}} // namespace hi::v1
