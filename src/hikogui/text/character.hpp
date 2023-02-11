


#pragma once

#include "text_phrasing.hpp"
#include "text_style.hpp"
#include "../i18n/iso_639.hpp"
#include "../unicode/grapheme.hpp"
#include <cstdint>

namespace hi {
inline namespace v1 {


struct character {
    using value_type = uint64_t;

    /**
     * [23: 0] grapheme only uses the lower 21 bits.
     * [31:24] phrasing
     * [47:32] iso-639 language
     * [63:48] text style.
     */
    value_type _value;

    constexpr character() noexcept = default;
    constexpr character(character const&) noexcept = default;
    constexpr character(character&&) noexcept = default;
    constexpr character& operator=(character const&) noexcept = default;
    constexpr character& operator=(character&&) noexcept = default;

    constexpr character(nullptr_t) noexcept : _value(0x1f'ffff) {}

    [[nodiscard]] constexpr hi::grapheme grapheme() const noexcept
    {
        return hi::grapheme{raw, _value & 0xff'ffff};
    }

    [[nodiscard]] constexpr hi::phrasing phrasing() const noexcept
    {
        return static_cast<hi::phrasing>((_value >> 24) & 0xff);
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{raw, (_value >> 32) & 0xffff}
    }

    [[nodiscard]] constexpr text_style style() const noexcept
    {
        return text_style{raw, (_value >> 48) & 0xffff};
    }


};




}}

