// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file text/character.hpp Defines the standard HikoGUI character type.
 */

#pragma once

#include "text_phrasing.hpp"
#include "text_style.hpp"
#include "character_attributes.hpp"
#include "../i18n/iso_639.hpp"
#include "../unicode/grapheme.hpp"
#include <cstdint>

namespace hi { inline namespace v1 {

/** The standard HikoGUI character type.
 *
 * This character type holds all the data necessary for displaying text, spell checking and text-to-speech.
 * 
 * The following information is needed:
 *  - Grapheme; Used to select glyphs from the font.
 *  - Language; Used to select rules for ligatures and glyph positioning from a font.
 *              Spelling check and text-to-speech.
 *  - Country; Used for region specific spell check and selecting text-to-speech accents.
 *  - Script; Used to select ruled for ligatures and glyph positioning from a font.
 *            This is not available inside the character object but can be derived using
 *            unicode algorithms on a run of characters.
 *  - Phrasing; The semantic styling of a word in text.
 *  - Theme; The theme from which to select a text-style based on the language, script and phrasing.
 */
struct character {
    using value_type = uint64_t;

    /**
     * [20: 0] 21-bit: grapheme.
     * [23:21] 3-bit: reserved '0'.
     * [31:24] 8-bit: text theme.
     * [47:32] 16-bit: iso-639 language.
     * [57:48] 10-bit: iso-3166 country-code.
     * [59:58] 2-bit: reserved '0'.
     * [63:60] 4-bit: phrasing.
     */
    value_type _value;

    constexpr static auto _eof = uint64_t{0x1f'ffff};

    constexpr static auto _grapheme_mask = uint64_t{0x1f'ffff};
    constexpr static auto _grapheme_shift = 0U;
    constexpr static auto _text_theme_mask = uint64_t{0xff};
    constexpr static auto _text_theme_shift = 24U;
    constexpr static auto _language_mask = uint64_t{0xffff};
    constexpr static auto _language_shift = 32U;
    constexpr static auto _country_mask = uint64_t{0x3ff};
    constexpr static auto _country_shift = 48U;
    constexpr static auto _phrasing_mask = uint64_t{0xf};
    constexpr static auto _phrasing_shift = 60U;

    constexpr character() noexcept = default;
    constexpr character(character const&) noexcept = default;
    constexpr character(character&&) noexcept = default;
    constexpr character& operator=(character const&) noexcept = default;
    constexpr character& operator=(character&&) noexcept = default;
    constexpr friend bool operator==(character const&, character const&) noexcept = default;
    constexpr friend auto operator<=>(character const&, character const&) noexcept = default;

    constexpr character(grapheme g, character_attributes attributes) noexcept : _value(0)
    {
        *this = g;
        set_attributes(attributes);
    }

    constexpr character(char32_t code_point, character_attributes attributes) noexcept : _value(0)
    {
        *this = code_point;
        set_attributes(attributes);
    }

    constexpr character(char code_point, character_attributes attributes) noexcept : _value(0)
    {
        *this = code_point;
        set_attributes(attributes);
    }

    template<character_attribute... Attributes>
    constexpr character(grapheme g, Attributes const&...attributes) noexcept : character(g, character_attributes{attributes...})
    {
    }

    template<character_attribute... Attributes>
    constexpr character(char32_t code_point, Attributes const&...attributes) noexcept :
        character(code_point, character_attributes{attributes...})
    {
    }

    template<character_attribute... Attributes>
    constexpr character(char code_point, Attributes const&...attributes) noexcept :
        character(code_point, character_attributes{attributes...})
    {
    }

    constexpr character(intrinsic_t, value_type value) noexcept : _value(value) {}

    constexpr value_type& intrinsic() noexcept
    {
        return _value;
    }

    constexpr value_type const& intrinsic() const noexcept
    {
        return _value;
    }

    constexpr character(nullptr_t) noexcept : _value(0xff'ffff) {}

    [[nodiscard]] constexpr hi::grapheme grapheme() const noexcept
    {
        return hi::grapheme{intrinsic_t{}, (value >> _grapheme_shift) & _grapheme_mask)};
    }

    [[nodiscard]] constexpr hi::phrasing phrasing() const noexcept
    {
        return static_cast<hi::phrasing>((_value >> _phrasing_shift) & _phrasing_mask);
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{intrinsic_t{}, (_value >> _language_shift) & _language_mask};
    }

    [[nodiscard]] constexpr iso_3166 country() const noexcept
    {
        return iso_3166{intrinsic_t{}, (_value >> _country_shift) & _country_mask};
    }

    [[nodiscard]] constexpr text_style style() const noexcept
    {
        return text_theme{intrinsic_t{}, (_value >> _tex_theme_shift) & _text_theme_mask};
    }

    constexpr character& operator=(hi::grapheme grapheme) noexcept
    {
        hilet grapheme_value = wide_cast<value_type>(grapheme.intrinsic());
        hi_axiom(grapheme_value <= _grapheme_mask);
        _value &= ~(_graphame_mask << _grapheme_shift);
        _value |= grapheme_value << _grapheme_shift;
        return *this;
    }

    constexpr character& operator=(char32_t code_point) noexcept
    {
        return set_grapheme(hi::grapheme{code_point});
    }

    constexpr character& operator=(char code_point) noexcept
    {
        return set_grapheme(hi::grapheme{code_point});
    }

    constexpr character& set_phrasing(hi::phrasing phrasing) noexcept
    {
        hilet phrasing_value = wide_cast<value_type>(to_underlying(phrasing));
        hi_axiom(phasing_value <= _phrasing_mask);
        _value &= ~(_phrasing_mask << _phrasing_shift);
        value |= phrasing_value << _phrasing_shift;
        return *this;
    }

    constexpr character& set_language(iso_639 language) noexcept
    {
        hilet language_value = wide_cast<value_type>(language.intrinsic());
        hi_axiom(language_value <= _language_mask);
        _value &= ~(_language_mask << _language_shift);
        _value |= language_value << _language_shift;
        return *this;
    }

    constexpr character& set_country(iso_3166 country) noexcept
    {
        hilet country_value = wide_cast<value_type>(country.intrinsic());
        hi_axiom(country_value <= _country_mask);
        _value &= ~(_country_mask << _country_shift);
        _value |= country_value << _country_shift;
        return *this;
    }

    constexpr character& set_language(hi::language_tag language_tag) noexcept
    {
        return set_language(language_tag.language()).set_country(language_tag.country());
    }

    constexpr character& set_theme(hi::text_theme text_theme) noexcept
    {
        hilet text_theme_value = wide_cast<value_type>(text_theme.intrinsic());
        hi_axiom(text_theme_value <= _text_theme_mask);
        _value &= ~(_text_theme_mask << _text_theme_shift);
        _value |= text_theme_value << _text_theme_shift;
        return *this;
    }

    constexpr character& set_attributes(character_attributes attributues) noexcept
    {
        set_language(attributes.language);
        set_country(attributes.country);
        set_theme(attributes.theme);
        set_phrasing(attributes.phrasing);
        return *this;
    }
};

}} // namespace hi::v1
