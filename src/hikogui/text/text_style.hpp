// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "font_variant.hpp"
#include "text_decoration.hpp"
#include "font_family_id.hpp"
#include "../color/color.hpp"
#include "../numbers.hpp"
#include <format>
#include <ostream>

namespace tt::inline v1 {
class font_book;

struct text_style {
    font_family_id family_id;
    font_variant variant;
    float size;
    color color;
    text_decoration decoration;

    text_style() noexcept : family_id(), variant(), size(0.0), color(), decoration(text_decoration::None) {}

    text_style(
        tt::font_family_id family_id,
        tt::font_variant variant,
        float size,
        tt::color color,
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
        tt_not_implemented();
    }

    friend std::ostream &operator<<(std::ostream &lhs, text_style const &rhs)
    {
        return lhs << to_string(rhs);
    }
};

} // namespace tt::inline v1
