// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file text/character.hpp Defines the standard HikoGUI character type.
 */

#pragma once

#include "../i18n/iso_639.hpp"
#include "../unicode/grapheme.hpp"
#include "../utility/module.hpp"
#include "text_phrasing.hpp"
#include "text_theme.hpp"
#include "character_attributes.hpp"
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
     * [33:21] 13-bit: text theme.
     * [37:34]  4-bit: phrasing.
     * [47:38] 10-bit: iso-3166 country-code.
     * [63:48] 16-bit: iso-639 language.
     */
    value_type _value;

    constexpr static auto _eof = uint64_t{0x1f'ffff};

    constexpr static auto _grapheme_mask = uint64_t{0x1f'ffff};
    constexpr static auto _grapheme_shift = 0U;
    constexpr static auto _text_theme_mask = uint64_t{0x1fff};
    constexpr static auto _text_theme_shift = 21U;
    constexpr static auto _phrasing_mask = uint64_t{0xf};
    constexpr static auto _phrasing_shift = 34U;
    constexpr static auto _country_mask = uint64_t{0x3ff};
    constexpr static auto _country_shift = 38U;
    constexpr static auto _language_mask = uint64_t{0xffff};
    constexpr static auto _language_shift = 48U;

    constexpr character() noexcept = default;
    constexpr character(character const&) noexcept = default;
    constexpr character(character&&) noexcept = default;
    constexpr character& operator=(character const&) noexcept = default;
    constexpr character& operator=(character&&) noexcept = default;
    constexpr friend bool operator==(character const&, character const&) noexcept = default;
    constexpr friend auto operator<=>(character const&, character const&) noexcept = default;

    constexpr character(hi::grapheme g, character_attributes attributes) noexcept : _value(0)
    {
        // [20: 0] 21-bit: grapheme.
        // [33:21] 13-bit: text theme.
        // [37:34]  4-bit: phrasing.
        // [47:38] 10-bit: iso-3166 country-code.
        // [63:48] 16-bit: iso-639 language.
        auto tmp = value_type{};
        tmp |= attributes.language.intrinsic();
        tmp <<= 10;
        tmp |= attributes.region.intrinsic();
        tmp <<= 4;
        tmp |= to_underlying(attributes.phrasing);
        tmp <<= 13;
        tmp |= attributes.theme.intrinsic();
        tmp <<= 21;
        tmp |= g.intrinsic();
        _value = tmp;
    }

    constexpr character(char32_t code_point, character_attributes attributes) noexcept :
        character(hi::grapheme{code_point}, attributes)
    {
    }

    constexpr character(char code_point, character_attributes attributes) noexcept :
        character(hi::grapheme{code_point}, attributes)
    {
    }

    template<character_attribute... Attributes>
    constexpr character(hi::grapheme g, Attributes const&...attributes) noexcept :
        character(g, character_attributes{attributes...})
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
        return hi::grapheme{intrinsic_t{}, (_value >> _grapheme_shift) & _grapheme_mask};
    }

    constexpr character& operator=(hi::grapheme grapheme) noexcept
    {
        hilet grapheme_value = wide_cast<value_type>(grapheme.intrinsic());
        hi_axiom(grapheme_value <= _grapheme_mask);
        _value &= ~(_grapheme_mask << _grapheme_shift);
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

    [[nodiscard]] constexpr hi::grapheme grapheme() const noexcept
    {
        return hi::grapheme{intrinsic_t{}, (_value >> _grapheme_shift) & _grapheme_mask};
    }

    constexpr character& set_grapheme(hi::grapheme grapheme) noexcept
    {
        hilet grapheme_value = wide_cast<value_type>(grapheme.intrinsic());
        _value &= ~(_grapheme_mask << _grapheme_shift);
        _value |= grapheme_value << _grapheme_shift;
        return *this;
    }

    [[nodiscard]] constexpr hi::text_phrasing phrasing() const noexcept
    {
        return static_cast<hi::text_phrasing>((_value >> _phrasing_shift) & _phrasing_mask);
    }

    constexpr character& set_phrasing(hi::text_phrasing phrasing) noexcept
    {
        hilet phrasing_value = wide_cast<value_type>(to_underlying(phrasing));
        hi_axiom(phrasing_value <= _phrasing_mask);
        _value &= ~(_phrasing_mask << _phrasing_shift);
        _value |= phrasing_value << _phrasing_shift;
        return *this;
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{intrinsic_t{}, (_value >> _language_shift) & _language_mask};
    }

    constexpr character& set_language(iso_639 language) noexcept
    {
        hilet language_value = wide_cast<value_type>(language.intrinsic());
        hi_axiom(language_value <= _language_mask);
        _value &= ~(_language_mask << _language_shift);
        _value |= language_value << _language_shift;
        return *this;
    }

    [[nodiscard]] constexpr iso_3166 country() const noexcept
    {
        return iso_3166{intrinsic_t{}, (_value >> _country_shift) & _country_mask};
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
        return set_language(language_tag.language).set_country(language_tag.region);
    }

    [[nodiscard]] constexpr text_theme theme() const noexcept
    {
        return text_theme{intrinsic_t{}, (_value >> _text_theme_shift) & _text_theme_mask};
    }

    constexpr character& set_theme(hi::text_theme text_theme) noexcept
    {
        hilet text_theme_value = wide_cast<value_type>(text_theme.intrinsic());
        hi_axiom(text_theme_value <= _text_theme_mask);
        _value &= ~(_text_theme_mask << _text_theme_shift);
        _value |= text_theme_value << _text_theme_shift;
        return *this;
    }

    [[nodiscard]] constexpr character_attributes attributes() const noexcept
    {
        // [20: 0] 21-bit: grapheme.
        // [33:21] 13-bit: text theme.
        // [37:34]  4-bit: phrasing.
        // [47:38] 10-bit: iso-3166 country-code.
        // [63:48] 16-bit: iso-639 language.
        auto tmp = _value;
        tmp >>= 21;
        hilet theme = text_theme{intrinsic_t{}, tmp & _text_theme_mask};
        tmp >>= 13;
        hilet phrasing = static_cast<text_phrasing>(tmp & _phrasing_mask);
        tmp >>= 4;
        hilet region = iso_3166{intrinsic_t{}, tmp & _country_mask};
        tmp >>= 10;
        hilet language = iso_639{intrinsic_t{}, tmp};
        return character_attributes{language, region, phrasing, theme};
    }

    constexpr character& set_attributes(character_attributes attributes) noexcept
    {
        // [20: 0] 21-bit: grapheme.
        // [33:21] 13-bit: text theme.
        // [37:34]  4-bit: phrasing.
        // [47:38] 10-bit: iso-3166 country-code.
        // [63:48] 16-bit: iso-639 language.
        auto tmp = value_type{};
        tmp |= attributes.language.intrinsic();
        tmp <<= 10;
        tmp |= attributes.region.intrinsic();
        tmp <<= 4;
        tmp |= to_underlying(attributes.phrasing);
        tmp <<= 13;
        tmp |= attributes.theme.intrinsic();
        tmp <<= 21;
        tmp |= grapheme();
        _value = tmp;
        return *this;
    }
};

}} // namespace hi::v1
