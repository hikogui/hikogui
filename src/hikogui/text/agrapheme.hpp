

#pragma once

#include "../unicode/grapheme.hpp"
#include "../i18n/iso_639.hpp"
#include "text_phrasing.hpp"
#include "text_style.hpp"
#include <bit>

namespace hi::inline v1 {

struct agrapheme {
    using value_type = uint64_t;

    /** Internal value.
     *
     *  - [63:43] grapheme
     *  - [42:39] phrasing
     *  - [31:16] ISO 639 language
     *  - [15:0] text-style
     */
    value_type _value;

    [[nodiscard]] friend bool operator==(agrapheme const &lhs, agrapheme const &rhs) noexcept = default;

    [[nodiscard]] friend auto operator<=>(agrapheme const &lhs, agrapheme const &rhs) noexcept
    {
        if (hilet r = lhs.grapheme() <=> rhs.grapheme(); r != std::strong_ordering::equal) {
            return r;
        }
        if (hilet r = lhs.phrasing() <=> rhs.phrasing(); r != std::strong_ordering::equal) {
            return r;
        }
        if (hilet r = lhs.language() <=> rhs.language(); r != std::strong_ordering::equal) {
            return r;
        }
        return lhs.style() <=> rhs.style();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return grapheme().empty();
    }

    explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] hi::grapheme grapheme() const noexcept
    {
        return std::bit_cast<hi::grapheme>(truncate<uint32_t>(_value >> 43));
    }

    [[nodiscard]] text_phrasing phrasing() const noexcept
    {
        return static_cast<text_phrasing>((_value >> 39) & 0xf);
    }

    [[nodiscard]] iso_639 language() const noexcept
    {
        return std::bit_cast<iso_639>(truncate<uint16_t>(_value >> 16));
    }

    [[nodiscard]] text_style style() const noexcept
    {
        return std::bit_cast<text_style>(truncate<uint16_t>(_value));
    }

    [[nodiscard]] text_sub_style sub_style(iso_15924 script = iso_15924{}) const noexcept
    {
        return style().sub_style(phrasing(), language(), script);
    }
};

}

