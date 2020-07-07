// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/text/UnicodeRanges.hpp"
#include "ttauri/text/GlyphID.hpp"
#include "ttauri/text/FontWeight.hpp"
#include "ttauri/text/FontVariant.hpp"
#include "ttauri/foundation/exceptions.hpp"
#include "ttauri/foundation/required.hpp"

namespace tt {

struct FontDescription {
    std::string family_name;
    std::string sub_family_name;

    bool monospace = false;
    bool serif = false;
    bool italic = false;
    bool condensed = false;
    FontWeight weight = FontWeight::Regular;
    float optical_size = 12.0;

    UnicodeRanges unicode_ranges;

    float xHeight = 0.0;
    float HHeight = 0.0;


    [[nodiscard]] FontVariant font_variant() const noexcept {
        return {weight, italic};
    }

    [[nodiscard]] friend std::string to_string(FontDescription const &rhs) noexcept {
        return fmt::format("{} - {}: {}{}{}{}{} {} {}",
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

    friend std::ostream &operator<<(std::ostream &lhs, FontDescription const &rhs) {
        return lhs << to_string(rhs);
    }
};


}
