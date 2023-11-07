// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/cp_1252.hpp Definition of the CP-1252 / Windows-1252 character map.
 * @ingroup char_maps
 */

module;
#include "../macros.hpp"

#include <cstdint>
#include <utility>
#include <array>
#include <bit>
#include <compare>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

export module hikogui_char_maps_cp_1252;
import hikogui_char_maps_char_converter;
import hikogui_utility;

export namespace hi { inline namespace v1 {

namespace detail {

constexpr auto cp_1252_make_table_0000_02DC() noexcept
{
    auto r = std::array<uint8_t, 0x02dd>{};
    for (auto i = 0x0; i != 0x2dd; ++i) {
        r[i] = 0x3f; // '?'
    }
    for (auto i = 0_uz; i != 0x80; ++i) {
        r[i] = char_cast<uint8_t>(i);
    }
    r[0x81] = 0x81;
    r[0x8d] = 0x8d;
    r[0x8f] = 0x8f;
    r[0x90] = 0x90;
    r[0x9d] = 0x9d;
    for (auto i = 0xa0; i != 0x100; ++i) {
        r[i] = char_cast<uint8_t>(i);
    }

    r[0x192] = 0x83;
    r[0x2c6] = 0x88;
    r[0x160] = 0x8a;
    r[0x152] = 0x8c;
    r[0x17d] = 0x8e;

    r[0x2dc] = 0x98;
    r[0x161] = 0x9a;
    r[0x153] = 0x9c;
    r[0x17e] = 0x9e;
    r[0x178] = 0x9f;

    return r;
}

constexpr auto cp_1252_make_table_2000_2122() noexcept
{
    auto r = std::array<uint8_t, 0x123>{};
    for (auto i = 0x0; i != 0x123; ++i) {
        r[i] = 0x3f; // '?'
    }

    r[0xac] = 0x80;
    r[0x1a] = 0x82;
    r[0x1e] = 0x84;
    r[0x26] = 0x85;
    r[0x20] = 0x86;
    r[0x21] = 0x87;
    r[0x30] = 0x89;
    r[0x39] = 0x8b;

    r[0x18] = 0x91;
    r[0x19] = 0x92;
    r[0x1c] = 0x93;
    r[0x1d] = 0x94;
    r[0x22] = 0x95;
    r[0x13] = 0x96;
    r[0x14] = 0x97;
    r[0x122] = 0x99;
    r[0x3a] = 0x9b;

    return r;
}

} // namespace detail

/** CP-1252 / Windows-1252 character map.
 * @ingroup char_maps
 */
template<>
struct char_map<"cp-1252"> {
    using char_type = char;

    [[nodiscard]] constexpr std::endian guess_endian(void const *ptr, size_t size, std::endian endian) const noexcept
    {
        return std::endian::native;
    }

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read(It& it, EndIt last) const noexcept
    {
        // clang-format off
        hi_axiom(it != last);
        hilet c = char_cast<char8_t>(*it++);
        switch (c) {
        case 0x80: return {0x20ac, true};
        case 0x81: return {0x81, true};
        case 0x82: return {0x201a, true};
        case 0x83: return {0x0192, true};
        case 0x84: return {0x201e, true};
        case 0x85: return {0x2026, true};
        case 0x86: return {0x2020, true};
        case 0x87: return {0x2021, true};
        case 0x88: return {0x02c6, true};
        case 0x89: return {0x2030, true};
        case 0x8a: return {0x0160, true};
        case 0x8b: return {0x2039, true};
        case 0x8c: return {0x0152, true};
        case 0x8d: return {0x8d, true};
        case 0x8e: return {0x017d, true};
        case 0x8f: return {0x8f, true};

        case 0x90: return {0x90, true};
        case 0x91: return {0x2018, true};
        case 0x92: return {0x2019, true};
        case 0x93: return {0x201c, true};
        case 0x94: return {0x201d, true};
        case 0x95: return {0x2022, true};
        case 0x96: return {0x2013, true};
        case 0x97: return {0x2014, true};
        case 0x98: return {0x02dc, true};
        case 0x99: return {0x2122, true};
        case 0x9a: return {0x0161, true};
        case 0x9b: return {0x203a, true};
        case 0x9c: return {0x0153, true};
        case 0x9d: return {0x9d, true};
        case 0x9e: return {0x017e, true};
        case 0x9f: return {0x0178, true};
        default: return {c, true};
        }
        // clang-format on
    }

    constexpr static auto range_0000_02DC = detail::cp_1252_make_table_0000_02DC();
    constexpr static auto range_2000_2122 = detail::cp_1252_make_table_2000_2122();

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x2dd) {
            if (code_point == 0x3f) {
                return {uint8_t{1}, true};
            } else {
                return {uint8_t{1}, range_0000_02DC[code_point] != 0x3f};
            }
        } else if (code_point < 0x2000) {
            return {uint8_t{1}, false};

        } else if (code_point < 0x2123) {
            return {uint8_t{1}, range_2000_2122[wide_cast<size_t>(code_point) - 0x2000] != 0x3f};

        } else {
            return {uint8_t{1}, false};
        }
    }

    template<typename It>
    constexpr void write(char32_t code_point, It& dst) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x2dd) {
            *dst++ = char_cast<char_type>(range_0000_02DC[code_point]);

        } else if (code_point < 0x2000) {
            *dst++ = char_cast<char_type>(0x3f);

        } else if (code_point < 0x2123) {
            *dst++ = char_cast<char_type>(range_2000_2122[code_point - 0x2000]);

        } else {
            *dst++ = char_cast<char_type>(0x3f);
        }
    }

#if defined(HI_HAS_SSE2)
    template<typename It>
    hi_force_inline __m128i read_ascii_chunk16(It it) const noexcept
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(std::addressof(*it)));
    }

    template<typename It>
    hi_force_inline void write_ascii_chunk16(__m128i chunk, It dst) const noexcept
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(std::addressof(*dst)), chunk);
    }
#endif
};

}} // namespace hi::v1
