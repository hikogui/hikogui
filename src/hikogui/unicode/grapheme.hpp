// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"
#include "../i18n/module.hpp"
#include "../strings.hpp"
#include "../stable_set.hpp"
#include "../log.hpp"
#include "unicode_normalization.hpp"
#include "ucd_general_categories.hpp"
#include "phrasing.hpp"
#include <cstdint>
#include <string>
#include <string_view>
#include <cstddef>
#include <memory>
#include <vector>
#include <algorithm>
#include <bit>

namespace hi::inline v1 {
namespace detail {

inline auto long_graphemes = ::hi::stable_set<std::u32string>{};

} // namespace detail

struct composed_t {};

/** A grapheme-cluster, what a user thinks a character is.
 *
 * A grapheme should not include typographical ligatures such as 'fi' as
 * the font should handle creating ligatures.
 *
 * If a grapheme is initialized with more than 1 code-points a long_grapheme
 * is allocated. This grapheme is never deleted from memory.
 *
 * This class is trivial and constant-destructible so that it can be used
 * as a character class in `std::basic_string` and used as a non-type template parameter.
 */
struct grapheme {
    using value_type = uint64_t;

    /** The grapheme's value.
     *
     * This class will hold:
     *  - [20: 0] U+0000 to U+10ffff single code-point,
     *            0x110000 to 0x1fffff index into long_grapheme table.
     *  - [35:21] ISO-639 language-code: 0 is wildcard.
     *  - [45:36] ISO-15924 script-code: 0 is wildcard.
     *  - [55:46] ISO-3166 region-code: 0 is wildcard.
     *  - [61:56] phrasing
     *  - [62:62] reserved = 0
     *  - [63:63] If bit is set this is a end-of-file.
     */
    value_type _value;

    constexpr grapheme() noexcept = default;
    constexpr grapheme(grapheme const&) noexcept = default;
    constexpr grapheme(grapheme&&) noexcept = default;
    constexpr grapheme& operator=(grapheme const&) noexcept = default;
    constexpr grapheme& operator=(grapheme&&) noexcept = default;

    constexpr grapheme(intrinsic_t, value_type value) : _value(value) {}

    constexpr value_type& intrinsic() noexcept
    {
        return _value;
    }

    constexpr value_type const& intrinsic() const noexcept
    {
        return _value;
    }

    /** Encode a single code-point.
     */
    constexpr grapheme(char32_t code_point) noexcept : _value(char_cast<value_type>(code_point))
    {
        hi_axiom(code_point <= 0x10'ffff);
    }

    constexpr grapheme(char ascii_char) noexcept : _value(char_cast<value_type>(ascii_char))
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char32_t code_point) noexcept
    {
        _value = char_cast<value_type>(code_point);
        return *this;
    }

    /** Encode a single code-point.
     */
    constexpr grapheme& operator=(char ascii_char) noexcept
    {
        hi_axiom(ascii_char >= 0 and ascii_char <= 0x7f);
        _value = char_cast<value_type>(ascii_char);
        return *this;
    }

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The NFC/NKFC normalized and composed code-point of this grapheme.
     */
    constexpr grapheme(composed_t, std::u32string_view code_points) noexcept
    {
        switch (code_points.size()) {
        case 0:
            hi_no_default();

        case 1:
            _value = char_cast<value_type>(code_points[0]);
            break;

        default:
            hilet index = detail::long_graphemes.insert(std::u32string{code_points});
            if (index < 0x0f'0000) {
                _value = narrow_cast<value_type>(index + 0x11'0000);

            } else {
                [[unlikely]] hi_log_error_once(
                    "grapheme::error::too-many", "Too many long graphemes encoded, replacing with U+fffd");
                _value = char_cast<value_type>(U'\ufffd');
            }
        }
    }

    /** Encode a grapheme from a list of code-points.
     *
     * @param code_points The non-normalized list of code-points.
     */
    constexpr explicit grapheme(std::u32string_view code_points) noexcept :
        grapheme(composed_t{}, unicode_normalize(code_points, unicode_normalize_config::NFC()))
    {
    }

    /** Get the codepoint/index part of the grapheme.
     */
    [[nodiscard]] constexpr uint32_t index() const noexcept
    {
        return _value & 0x1f'ffff;
    }

    [[nodiscard]] constexpr iso_639 language() const noexcept
    {
        return iso_639{intrinsic_t{}, narrow_cast<uint16_t>((_value >> 21) & 0x7fff)};
    }

    constexpr void set_language(iso_639 rhs) noexcept
    {
        hi_axiom(rhs.intrinsic() <= 0x7fff);

        constexpr auto mask = ~(value_type{0x7fff} << 21);
        _value &= mask;
        _value |= wide_cast<value_type>(rhs.intrinsic()) << 21;
    }

    [[nodiscard]] constexpr iso_15924 script() const noexcept
    {
        return iso_15924{intrinsic_t{}, narrow_cast<uint16_t>((_value >> 36) & 0x3ff)};
    }

    constexpr void set_script(iso_15924 rhs) noexcept
    {
        hi_axiom(rhs.intrinsic() < 1000);

        constexpr auto mask = ~(value_type{0x3ff} << 36);
        _value &= mask;
        _value |= wide_cast<value_type>(rhs.intrinsic()) << 36;
    }

    [[nodiscard]] constexpr iso_3166 region() const noexcept
    {
        return iso_3166{intrinsic_t{}, narrow_cast<uint16_t>((_value >> 46) & 0x3ff)};
    }

    constexpr void set_region(iso_3166 rhs) noexcept
    {
        hi_axiom(rhs.intrinsic() < 1000);

        constexpr auto mask = ~(value_type{0x3ff} << 46);
        _value &= mask;
        _value |= wide_cast<value_type>(rhs.intrinsic()) << 46;
    }

    [[nodiscard]] constexpr hi::language_tag language_tag() const noexcept
    {
        auto tmp = _value;
        tmp >>= 21;
        hilet language_ = iso_639{intrinsic_t{}, narrow_cast<uint16_t>(tmp & 0x7fff)};
        tmp >>= 15;
        hilet script_ = iso_15924{intrinsic_t{}, narrow_cast<uint16_t>(tmp & 0x3ff)};
        tmp >>= 15;
        hilet region_ = iso_3166{intrinsic_t{}, narrow_cast<uint16_t>(tmp & 0x3ff)};
        return hi::language_tag{language_, script_, region_};
    }

    constexpr void set_language_tag(hi::language_tag rhs) noexcept
    {
        hi_axiom(rhs.region.intrinsic() <= 0x7fff);
        hi_axiom(rhs.script.intrinsic() < 1000);
        hi_axiom(rhs.language.intrinsic() < 1000);

        auto tmp = wide_cast<value_type>(rhs.region.intrinsic());
        tmp <<= 10;
        tmp |= rhs.script.intrinsic();
        tmp <<= 15;
        tmp |= rhs.language.intrinsic();
        tmp <<= 21;

        constexpr auto mask = ~(uint64_t{0x7'ffff} << 21);
        _value &= mask;
        _value |= tmp;
    }

    [[nodiscard]] constexpr hi::phrasing phrasing() const noexcept
    {
        return static_cast<hi::phrasing>((_value >> 56) & 0x3f);
    }

    constexpr void set_phrasing(hi::phrasing rhs) noexcept
    {
        hi_axiom(std::to_underlying(rhs) <= 0x3f);

        constexpr auto mask = ~(value_type{0x3f} << 56);
        _value &= mask;
        _value |= static_cast<value_type>(rhs) << 56;
    }

    [[nodiscard]] std::u32string const& long_grapheme() const noexcept
    {
        hilet i = index();
        hi_axiom(i > 0x10'ffff and i <= 0x1f'ffff);
        return detail::long_graphemes[i - 0x11'0000];
    }

    /** Return the number of code-points encoded in the grapheme.
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept
    {
        return index() <= 0x10'ffff ? 1_uz : long_grapheme().size();
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behaviour to index beyond the number of encoded code-points.
     * @param i Index of code-point in the grapheme.
     * @return code-point at the given index.
     */
    [[nodiscard]] constexpr char32_t operator[](size_t i) const noexcept
    {
        if (hilet code_point = index(); code_point <= 0x10'ffff) {
            hi_axiom(i == 0);
            return char_cast<char32_t>(code_point);
        } else {
            hi_axiom_bounds(i, *this);
            return long_grapheme()[i];
        }
    }

    /** Get the code-point at the given index.
     *
     * @note It is undefined-behaviour to index beyond the number of encoded code-points.
     * @tparam I Index of code-point in the grapheme.
     * @param rhs The grapheme to query.
     * @return code-point at the given index.
     */
    template<size_t I>
    [[nodiscard]] friend constexpr char32_t get(grapheme const& rhs) noexcept
    {
        if (hilet code_point = rhs.index(); code_point <= 0x10'ffff) {
            hi_axiom(I == 0);
            return code_point;

        } else {
            hi_axiom_bounds(I, rhs);
            return rhs.long_grapheme()[I];
        }
    }

    /** Get a list of code-point normalized to NFC.
     */
    [[nodiscard]] constexpr std::u32string composed() const noexcept
    {
        if (hilet code_point = index(); code_point <= 0x10'ffff) {
            return std::u32string{char_cast<char32_t>(code_point)};

        } else {
            return long_grapheme();
        }
    }

    /** Get a list of code-point normalized to NFD.
     */
    [[nodiscard]] constexpr std::u32string
    decomposed(unicode_normalize_config config = unicode_normalize_config::NFD()) const noexcept
    {
        return unicode_decompose(composed(), config);
    }

    /** Compare equivalence of two graphemes.
     *
     * This comparision ignores language-tag and phrasing of a grapheme.
     */
    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, grapheme const& rhs) noexcept
    {
        return lhs.index() == rhs.index();
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        hi_axiom(char_cast<value_type>(rhs) <= 0x10'ffff);
        return lhs.index() == char_cast<value_type>(rhs);
    }

    [[nodiscard]] friend constexpr bool operator==(grapheme const& lhs, char const& rhs) noexcept
    {
        hi_axiom(char_cast<value_type>(rhs) <= 0x7f);
        return lhs.index() == char_cast<value_type>(rhs);
    }

    /** Compare two graphemes lexicographically.
     */
    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, grapheme const& rhs) noexcept
    {
        return lhs.decomposed() <=> rhs.decomposed();
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char32_t const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::strong_ordering operator<=>(grapheme const& lhs, char const& rhs) noexcept
    {
        return lhs <=> grapheme{rhs};
    }

    [[nodiscard]] friend constexpr std::string to_string(grapheme const& rhs) noexcept
    {
        return hi::to_string(rhs.composed());
    }

    [[nodiscard]] friend constexpr std::wstring to_wstring(grapheme const& rhs) noexcept
    {
        return hi::to_wstring(rhs.composed());
    }

    [[nodiscard]] friend constexpr std::u32string to_u32string(grapheme const& rhs) noexcept
    {
        return rhs.composed();
    }
};

} // namespace hi::inline v1

template<>
struct std::hash<hi::grapheme> {
    [[nodiscard]] std::size_t operator()(hi::grapheme const& rhs) const noexcept
    {
        return std::hash<hi::grapheme::value_type>{}(rhs._value);
    }
};
