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
 *  - region; Used for region specific spell check and selecting text-to-speech accents.
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
     * [60:21] 40-bit: attributes.
     * [62:61] reserved
     * [63:63] Sign-bit reserved for eof.
     */
    value_type _value;

    constexpr character() noexcept = default;
    constexpr character(character const&) noexcept = default;
    constexpr character(character&&) noexcept = default;
    constexpr character& operator=(character const&) noexcept = default;
    constexpr character& operator=(character&&) noexcept = default;
    constexpr friend bool operator==(character const&, character const&) noexcept = default;
    constexpr friend auto operator<=>(character const&, character const&) noexcept = default;

    constexpr character(hi::grapheme g, character_attributes attributes) noexcept :
        _value((attributes.intrinsic() << 21) | g.intrinsic())
    {
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

    constexpr character& operator=(hi::grapheme grapheme) noexcept
    {
        return set_grapheme(grapheme);
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
        return hi::grapheme{intrinsic_t{}, _value & 0x1f'ffff};
    }

    constexpr character& set_grapheme(hi::grapheme grapheme) noexcept
    {
        _value >>= 21;
        _value <<= 21;
        _value |= grapheme.intrinsic();
        return *this;
    }

    [[nodiscard]] size_t size() const noexcept
    {
        return grapheme().size();
    }

    [[nodiscard]] char32_t operator[](size_t i) const noexcept
    {
        return grapheme()[i];
    }

    [[nodiscard]] constexpr character_attributes attributes() const noexcept
    {
        return character_attributes{intrinsic_t{}, _value >> 21};
    }

    constexpr character& set_attributes(character_attributes attributes) noexcept
    {
        _value &= 0x1f'ffff;
        _value |= attributes.intrinsic() << 21;
        return *this;
    }
};

}} // namespace hi::v1
