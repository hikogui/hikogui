// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "unicode_ranges.hpp"
#include "glyph_id.hpp"
#include "font_weight.hpp"
#include "font_variant.hpp"
#include "../exception.hpp"
#include "../required.hpp"

namespace tt {

struct font_description {
    std::string family_name;
    std::string sub_family_name;

    bool monospace = false;
    bool serif = false;
    bool italic = false;
    bool condensed = false;
    font_weight weight = font_weight::Regular;
    float optical_size = 12.0;

    unicode_ranges unicode_ranges;

    float xHeight = 0.0;
    float HHeight = 0.0;
    float DigitWidth = 0.0;

    [[nodiscard]] font_variant font_variant() const noexcept {
        return {weight, italic};
    }

    [[nodiscard]] friend std::string to_string(font_description const &rhs) noexcept {
        return std::format("{} - {}: {}{}{}{}{} {} {}",
            rhs.family_name,
            rhs.sub_family_name,
            rhs.monospace ? 'M' : '_',
            rhs.serif ? 'S': '_',
            rhs.italic ? 'I': '_',
            rhs.condensed ? 'C': '_',
            to_char(rhs.weight),
            rhs.optical_size,
            rhs.unicode_ranges
        );
    }

    friend std::ostream &operator<<(std::ostream &lhs, font_description const &rhs) {
        return lhs << to_string(rhs);
    }
};

template<typename CharT>
struct std::formatter<tt::font_description, CharT> : std::formatter<std::string_view, CharT> {
    auto format(tt::font_description t, auto &fc)
    {
        return std::formatter<std::string_view, CharT>::format(to_string(t), fc);
    }
};


}
