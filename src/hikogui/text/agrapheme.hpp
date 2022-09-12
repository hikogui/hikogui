// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)


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

    [[nodiscard]] constexpr friend bool operator==(agrapheme const &lhs, agrapheme const &rhs) noexcept = default;

    [[nodiscard]] constexpr friend std::partial_ordering operator<=>(agrapheme const &lhs, agrapheme const &rhs) noexcept
    {
        return lhs.grapheme() <=> rhs.grapheme();
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        return grapheme().empty();
    }

    constexpr explicit operator bool() const noexcept
    {
        return not empty();
    }

    [[nodiscard]] size_t hash() const noexcept
    {
        return std::hash<value_type>{}(_value);
    }

    [[nodiscard]] constexpr hi::grapheme grapheme() const noexcept
    {
        return std::bit_cast<hi::grapheme>(truncate<uint32_t>(_value >> 43));
    }

    [[nodiscard]] constexpr text_phrasing phrasing() const noexcept
    {
        return static_cast<text_phrasing>((_value >> 39) & 0xf);
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return std::bit_cast<iso_639>(truncate<uint16_t>(_value >> 16));
    }

    [[nodiscard]] constexpr text_style style() const noexcept
    {
        return std::bit_cast<text_style>(truncate<uint16_t>(_value));
    }

    [[nodiscard]] text_sub_style sub_style(iso_15924 script = iso_15924{}) const noexcept
    {
        return style().sub_style(phrasing(), language(), script);
    }
};

}

template<>
struct std::hash<hi::agrapheme> {
    [[nodiscard]] size_t operator()(hi::agrapheme const &rhs) const noexcept
    {
        return rhs.hash();
    }
};
