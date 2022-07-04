// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_variant.hpp"
#include "text_decoration.hpp"
#include "font_family_id.hpp"
#include "text_phrasing.hpp"
#include "../color/color.hpp"
#include "../i18n/iso_15924.hpp"
#include "../i18n/iso_639.hpp"
#include "../numbers.hpp"
#include <format>
#include <ostream>

namespace hi::inline v1 {
class font_book;

class text_sub_style {
public:

    [[nodiscard]] bool matches(text_phrasing phrasing, iso_639 language, iso_15924 script) const noexcept
    {
        if (not to_bool(_phrasing_mask & to_text_phrasing_mask(phrasing)) {
            return false;
        }
        if (_language_filter and language and _language_filter != language) {
            return false;
        }
        if (_script_filter and script and _script_filter != script) {
            return false;
        }
        return true;
    }

private:
    text_phrasing_mask _phrasing_mask;
    iso_639 _language_filter;
    iso_15924 _script_filter;

    font_family_id _family_id;
    font_variant _variant;
    float _size;
    color _color;
    text_decoration _decoration;
};


struct text_style {
    font_family_id family_id;
    font_variant variant;
    float size;
    color color;
    text_decoration decoration;

    text_style() noexcept : family_id(), variant(), size(0.0), color(), decoration(text_decoration::None) {}

    text_style(
        hi::font_family_id family_id,
        hi::font_variant variant,
        float size,
        hi::color color,
        text_decoration decoration) noexcept :
        family_id(family_id), variant(variant), size(size), color(color), decoration(decoration)
    {
    }

    text_style(text_style const &) noexcept = default;
    text_style(text_style &&) noexcept = default;
    text_style &operator=(text_style const &) noexcept = default;
    text_style &operator=(text_style &&) noexcept = default;

    [[nodiscard]] friend std::string to_string(text_style const &rhs) noexcept
    {
        // XXX - fmt:: no longer can format tagged_ids??????

        // return std::format("<text_style id={},v={},s={},c={},d={}>",
        //    rhs.family_id, rhs.variant, rhs.size, rhs.color, rhs.decoration
        //);
        hi_not_implemented();
    }

    [[nodiscard]] float cap_height(font_book const &font_book) const noexcept;

    friend std::ostream &operator<<(std::ostream &lhs, text_style const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace hi::inline v1
