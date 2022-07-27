// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_encoding.hpp"

namespace hi::inline v1 {

template<>
struct char_encoder<"utf-32"> {
    using char_type = char32_t;

    [[nodiscard]] constexpr char_encoder_result read(char_type const *ptr, size_t size) const noexcept
    {
        hi_axiom(size != 0);

        if (auto cu = *ptr; cu < 0xd800) {
            return cu;

        } else if (cu < 0xe000) {
            // Surrogates
            return {0xfffd, 1, false};

        } else if (cu < 0x11'0000) {
            return cu;

        } else {
            // Out-of-range
            return {0xfffd, 1, false};
        }
    }

    template<bool Write>
    [[nodiscard]] constexpr char_encoder_result write(char32_t code_point, char_type *ptr, size_t size) const noexcept
    {
        hi_axiom(code_point <= 0x10'ffff);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        *ptr = code_point;
        return 1;
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
    {
        // Load the UTF-16 data.
        auto c0 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
        auto c1 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 4));
        auto c2 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 8));
        auto c3 = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 12));

        auto lo = _mm_packs_epi32(c0, c1);
        auto hi = _mm_packs_epi32(c2, c3);

        // To get _mm_packus_epi16() to work we need to prepare the data as follows:
        //  - bit 15 must be '0'.
        //  - if bit 15 was originally set than we need to set any of bits [14:8].

        // Positive numbers -> 0b0000'0000
        // Negative numbers -> 0b1000'0000
        auto sign_lo = _mm_sra_epi16(lo, 15);
        auto sign_hi = _mm_sra_epi16(hi, 15);
        auto sign = _mm_packs_epi16(sign_lo, sign_hi);

        // ASCII            -> 0b0ccc'cccc
        // positive numbers -> 0b1???'????
        // negative numbers -> 0b0000'0000
        auto chunk = _mm_packus_epi16(lo, hi);

        // ASCII            -> 0b0ccc'cccc
        // positive numbers -> 0b1???'????
        // negative numbers -> 0b1000'0000
        return _mm_or_si128(chunk, sign);
    }

    hi_force_inline void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
    {
        auto zero = _mm_setzero_si128();
        auto lo = _mm_unpacklo_epi8(chunk, zero);
        auto hi = _mm_unpackhi_epi8(chunk, zero);

        auto c0 = _mm_unpacklo_epi8(lo, zero);
        auto c1 = _mm_unpackhi_epi8(lo, zero);
        auto c2 = _mm_unpacklo_epi8(hi, zero);
        auto c3 = _mm_unpackhi_epi8(hi, zero);

        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), c0);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 4), c1);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 8), c2);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 12), c3);
    }
#endif
};

} // namespace hi::inline v1
