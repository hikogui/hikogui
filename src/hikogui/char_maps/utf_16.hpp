// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_converter.hpp"

namespace hi::inline v1 {

template<>
struct char_map<"utf-16"> {
    using char_type = char16_t;

    [[nodiscard]] constexpr char_map_result read(char_type const *ptr, size_t size) const noexcept
    {
        hi_axiom(size != 0);

        if (auto cu = *ptr; cu < 0xd800) {
            return {wide_cast<char32_t>(cu), 1};

        } else if (cu < 0xdc00) {
            auto cp = cu & 0x03ff;
            if (size < 2) {
                // first surrogate at end of string.
                return {0xfffd, 1, false};

            } else {
                cu = *++ptr;
                if (cu < 0xdc00) {
                    // unpaired surrogate.
                    return {0xfffd, 1, false};

                } else if (cu < 0xe000) {
                    cp <<= 10;
                    cp |= cu & 0x03ff;
                    cp += 0x01'0000;
                    return {cp, 2};

                } else {
                    // unpaired surrogate.
                    return {0xfffd, 1, false};
                }
            }
        } else if (cu < 0xe000) {
            // Invalid low surrogate.
            return {0xfffd, 1, false};

        } else {
            return {cu, 1};
        }
    }

    template<bool Write>
    [[nodiscard]] constexpr char_map_result write(char32_t code_point, char_type *ptr) const noexcept
    {
        hi_axiom(code_point <= 0x10'ffff);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (auto tmp = truncate<int32_t>(code_point) - 0x1'0000; tmp >= 0) {
            if constexpr (Write) {
                ptr[1] = truncate<char16_t>((tmp & 0x3ff) + 0xdc00);
                tmp >>= 10;
                ptr[0] = truncate<char16_t>(tmp + 0xd800);
            }
            return 2;

        } else {
            if constexpr (Write) {
                ptr[0] = truncate<char16_t>(code_point);
            }
            return 1;
        }
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
    {
        // Load the UTF-16 data.
        auto lo = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
        auto hi = _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr + 8));

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

        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), lo);
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr + 8), hi);
    }
#endif
};

} // namespace hi::inline v1
