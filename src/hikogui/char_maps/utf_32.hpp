// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_converter.hpp"

namespace hi::inline v1 {

template<>
struct char_map<"utf-32"> {
    using char_type = char32_t;

    [[nodiscard]] constexpr std::pair<char32_t, bool> read(char_type const *& ptr, char_type const *last) const noexcept
    {
        hi_axiom(ptr != last);

        if (auto cu = *ptr++; cu < 0xd800) {
            return {cu, true};

        } else if (cu < 0xe000) {
            // Surrogates
            return {0xfffd, false};

        } else if (cu < 0x11'0000) {
            return {cu, true};

        } else {
            // Out-of-range
            return {0xfffd, false};
        }
    }

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));
        return {uint8_t{1}, true};
    }

    constexpr void write(char32_t code_point, char_type *& ptr) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        *ptr++ = code_point;
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
    {
        // Load the UTF-16 data.
        hilet c0 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
        hilet c1 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 4));
        hilet c2 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 8));
        hilet c3 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 12));

        hilet lo = _mm_packs_epi32(c0, c1);
        hilet hi = _mm_packs_epi32(c2, c3);

        // To get _mm_packus_epi16() to work we need to prepare the data as follows:
        //  - bit 15 must be '0'.
        //  - if bit 15 was originally set than we need to set any of bits [14:8].

        // Positive numbers -> 0b0000'0000
        // Negative numbers -> 0b1000'0000
        hilet sign_lo = _mm_srai_epi16(lo, 15);
        hilet sign_hi = _mm_srai_epi16(hi, 15);
        hilet sign = _mm_packs_epi16(sign_lo, sign_hi);

        // ASCII            -> 0b0ccc'cccc
        // positive numbers -> 0b1???'????
        // negative numbers -> 0b0000'0000
        hilet chunk = _mm_packus_epi16(lo, hi);

        // ASCII            -> 0b0ccc'cccc
        // positive numbers -> 0b1???'????
        // negative numbers -> 0b1000'0000
        return _mm_or_si128(chunk, sign);
    }

    hi_force_inline void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
    {
        hilet zero = _mm_setzero_si128();
        hilet lo = _mm_unpacklo_epi8(chunk, zero);
        hilet hi = _mm_unpackhi_epi8(chunk, zero);

        hilet c0 = _mm_unpacklo_epi8(lo, zero);
        hilet c1 = _mm_unpackhi_epi8(lo, zero);
        hilet c2 = _mm_unpacklo_epi8(hi, zero);
        hilet c3 = _mm_unpackhi_epi8(hi, zero);

        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), c0);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 4), c1);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 8), c2);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 12), c3);
    }
#endif
};

} // namespace hi::inline v1
