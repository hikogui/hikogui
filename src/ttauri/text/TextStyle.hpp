// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/text/FontDescription.hpp"
#include "ttauri/text/TextDecoration.hpp"
#include "ttauri/text/FontFamilyID.hpp"
#include "ttauri/R16G16B16A16SFloat.hpp"
#include <fmt/format.h>
#include <ostream>

namespace tt {

struct TextStyle {
    static constexpr float default_dpi = 84.0f;
    static constexpr float dpi_scale = default_dpi / 72.0f;

    FontFamilyID family_id;
    FontVariant variant;
    float size;
    vec color;
    TextDecoration decoration;

    TextStyle() noexcept :
        family_id(), variant(), size(0.0), color(), decoration(TextDecoration::None) {}

    TextStyle(tt::FontFamilyID family_id, tt::FontVariant variant, float size, vec color, TextDecoration decoration) noexcept :
        family_id(family_id), variant(variant), size(size), color(color), decoration(decoration) {}

    TextStyle(std::string_view family_name, tt::FontVariant variant, float size, vec color, TextDecoration decoration) noexcept;

    TextStyle(TextStyle const &) noexcept = default;
    TextStyle(TextStyle &&) noexcept = default;
    TextStyle &operator=(TextStyle const &) noexcept = default;
    TextStyle &operator=(TextStyle &&) noexcept = default;

    float scaled_size() const noexcept {
        return size * dpi_scale;
    }

    [[nodiscard]] friend std::string to_string(TextStyle const &rhs) noexcept {
        // XXX - fmt:: no longer can format tagged_ids??????

        //return fmt::format("<TextStyle id={},v={},s={},c={},d={}>",
        //    rhs.family_id, rhs.variant, rhs.size, rhs.color, rhs.decoration
        //);
        tt_not_implemented;
    }

    friend std::ostream &operator<<(std::ostream &lhs, TextStyle const &rhs) {
        return lhs << to_string(rhs);
    }
};

}
