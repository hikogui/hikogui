// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/theme.hpp"
#include "TTauri/Foundation/UnicodeData.hpp"

namespace TTauri {

/** A font variant is one of 16 different fonts that can be part of a family.
* It only contains the font-weight and if it is italic/oblique.
*
* monospace, serif, condensed, expanded & optical-size are all part of the font family.
*/
class FontVariant {
    uint8_t value;

public:
    constexpr static size_t max() { return 16; }

    constexpr FontVariant(font_weight weight, bool italic) noexcept : value(static_cast<int>(weight) | (italic ? 8 : 0)) {}
    constexpr FontVariant() noexcept : FontVariant(font_weight::Regular, false) {}
    constexpr FontVariant(font_weight weight) noexcept : FontVariant(weight, false) {}
    constexpr FontVariant(bool italic) noexcept : FontVariant(font_weight::Regular, italic) {}

    constexpr font_weight weight() const noexcept {
        ttauri_assume(value < 16);
        return static_cast<font_weight>(value & 0x7);
    }

    [[nodiscard]] constexpr bool italic() const noexcept {
        ttauri_assume(value < 16);
        return (value & 0x8) != 0;
    }

    constexpr FontVariant &set_weight(font_weight weight) noexcept {
        ttauri_assume(value < 16);
        value &= 0x8;
        let weight_ = static_cast<uint8_t>(weight);
        ttauri_assume(weight_ < 8);
        value |= weight_;
        return *this;
    }

    constexpr FontVariant &set_italic(bool italic) noexcept {
        ttauri_assume(value < 16);
        value &= 0x7;
        value |= italic ? 8 : 0;
        return *this;
    }

    constexpr operator int () const noexcept {
        ttauri_assume(value < 16);
        return value;
    }

    /** Get an alternative font variant.
     * @param i 0 is current value, 1 is best alternative, 15 is worst alternative.
     */
    constexpr FontVariant alternative(int i) const noexcept {
        ttauri_assume(i >= 0 && i < 16);
        auto it = italic() == (i < 8);
        auto w = font_weight_alterative(weight(), i % 8);
        return {w, it};
    }
};

struct FontDescription {
    URL url;
    std::string family_name;
    std::string sub_family_name;

    bool monospace = false;
    bool serif = false;
    bool italic = false;
    bool condensed = false;
    font_weight weight = font_weight::Regular;
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