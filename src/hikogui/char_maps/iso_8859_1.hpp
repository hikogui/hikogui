// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/iso_8859_1.hpp Definition of the ISO-8859-1 / Latin-1 character map.
 * @ingroup char_maps
 */

#pragma once

#include "char_converter.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <cstdint>
#include <utility>
#include <bit>
#include <compare>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

hi_export_module(hikogui.char_maps.iso_8859_1);

hi_export namespace hi { inline namespace v1 {

/** ISO-8859-1 / Latin-1 character map.
 *
 * @ingroup char_maps
 */
template<>
struct char_map<"iso-8859-1"> {
    using char_type = char;

    [[nodiscard]] constexpr std::endian guess_endian(void const *ptr, size_t size, std::endian endian) const noexcept
    {
        return std::endian::native;
    }

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read(It& it, EndIt last) const noexcept
    {
        hi_axiom(it != last);
        return {char_cast<char32_t>(*it++), true};
    }

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x0100) {
            return {uint8_t{1}, true};

        } else {
            return {uint8_t{1}, false};
        }
    }

    template<typename It>
    constexpr void write(char32_t code_point, It& dst) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (code_point < 0x0100) {
            *dst++ = char_cast<char>(code_point);
        } else {
            *dst++ = '?';
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
