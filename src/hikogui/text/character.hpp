// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "text_phrasing.hpp"
#include "text_style.hpp"
#include "../i18n/iso_639.hpp"
#include "../unicode/grapheme.hpp"
#include <cstdint>

namespace hi { inline namespace v1 {

struct character {
    using value_type = uint64_t;

    /**
     * [23: 0] grapheme only uses the lower 24 bits.
     * [31:24] phrasing
     * [47:32] iso-639 language
     * [63:48] text theme.
     */
    value_type _value;

    constexpr auto _grapheme_mask = uint64_t{0x0000'0000'00'ffffffULL};
    constexpr auto _grapheme_shift = 0U;
    constexpr auto _phrasing_mask = uint64_t{0x0000'0000'ff'000000ULL};
    constexpr auto _phrasing_shift = 24U;
    constexpr auto _language_mask = uint64_t{0x0000'ffff'00'000000ULL};
    constexpr auto _language_shift = 32U;
    constexpr auto _theme_mask = uint64_t{0xffff'0000'00'000000ULL};
    constexpr auto _theme_shift = 48U;

    constexpr character() noexcept = default;
    constexpr character(character const&) noexcept = default;
    constexpr character(character&&) noexcept = default;
    constexpr character& operator=(character const&) noexcept = default;
    constexpr character& operator=(character&&) noexcept = default;
    constexpr friend bool operator==(character const&, character const&) noexcept = default;
    constexpr friend auto operator<=>(character const&, character const&) noexcept = default;

    constexpr character(intrinsic_t, value_type value) noexcept : _value(value) {}

    constexpr value_type &intrinsic() noexcept
    {
        return _value;
    }

    constexpr value_type const &intrinsic() const noexcept
    {
        return _value;
    }

    constexpr character(nullptr_t) noexcept : _value(0xff'ffff) {}

    [[nodiscard]] constexpr hi::grapheme grapheme() const noexcept
    {
        return hi::grapheme{intrinsic, (_value & _grapheme_mask) >> _grapheme_shift)};
    }

    [[nodiscard]] constexpr hi::phrasing phrasing() const noexcept
    {
        return static_cast<hi::phrasing>((_value & _phrasing_mask) >> _phrasing_shift);
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{intrinsic, (_value & _language_mask) >> _language_shift};
    }

    [[nodiscard]] constexpr text_style style() const noexcept
    {
        return text_theme{intrinsic, (_value & _theme_mask) >> _theme_shift};
    }

    constexpr character& set_grapheme(hi::grapheme grapheme) noexcept
    {
        hilet grapheme_value = wide_cast<value_type>(grapheme.intrinsic());
        hi_axiom(grapheme_value <= 0xff'ffff);
        _value &= ~_graphame_mask;
        _value |= grapheme_value << _grapheme_shift;
        return *this;
    }

    constexpr character& set_phrasing(hi::phrasing phrasing) noexcept
    {
        hilet phrasing_value = wide_cast<value_type>(to_underlying(phrasing));
        hi_axiom(phasing_value <= 0xff);
        _value &= ~_phrasing_mask;
        value |= phrasing_value << _phrasing_shift;
        return *this;
    }

    constexpr character& set_language(iso_639 language) noexcept
    {
        hilet language_value = wide_cast<value_type>(language.intrinsic());
        hi_axiom(language_value <= 0xffff);
        _value &= ~_language_mask;
        _value |= language_value << _language_shift;
        return *this;
    }

    constexpr character& set_theme(text_theme theme) noexcept
    {
        hilet theme_value = wide_cast<value_type>(theme.intrinsic());
        hi_axiom(theme_value <= 0xffff);
        _value &= ~_theme_mask;
        _value |= theme_value << _theme_shift;
        return *this;
    }
};

}} // namespace hi::v1
