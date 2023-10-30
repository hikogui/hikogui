// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/utf_16.hpp Definition of the Unicode UTF-16 encoding.
 */

module;
#include "../macros.hpp"

#include <bit>
#include <cstdint>
#include <utility>
#include <compare>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

export module hikogui_char_maps_utf_16;
import hikogui_char_maps_char_converter;
import hikogui_utility;

hi_warning_push();
// C26490: Don't use reinterpret_cast.
// Needed for SIMD intrinsics.
hi_warning_ignore_msvc(26490);

export namespace hi { inline namespace v1 {

/** Unicode UTF-16 encoding.
 *
 * @ingroup char_maps
 */
template<>
struct char_map<"utf-16"> {
    using char_type = char16_t;

    [[nodiscard]] std::endian guess_endian(void const *ptr, size_t size, std::endian endian) const noexcept
    {
        hi_assert_not_null(ptr);
        auto *ptr_ = static_cast<uint8_t const *>(ptr);
        hi_axiom_not_null(ptr_);

        if (size < 2) {
            return std::endian::native;
        } else {
            // Check for BOM.
            if (ptr_[0] == 0xfe and ptr_[1] == 0xff) {
                return std::endian::big;
            } else if (ptr_[0] == 0xff and ptr_[1] == 0xfe) {
                return std::endian::little;
            }

            // Check for sequences of zeros.
            auto count = std::array<size_t, 2>{};
            for (auto i = 0; i != size; ++i) {
                count[i % 2] = ptr_[i] == 0 ? count[i % 2] + 1 : 0;
                if (count[i % 2] >= 8) {
                    return i % 2 == 0 ? std::endian::big : std::endian::little;
                }
            }

            return endian;
        }
    }

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read(It& it, EndIt last) const noexcept
    {
        hi_axiom(it != last);

        if (auto cu = *it++; cu < 0xd800) {
            return {char_cast<char32_t>(cu), true};

        } else if (cu < 0xdc00) {
            if (it == last) {
                // first surrogate at end of string.
                return {0xfffd, false};

            } else {
                auto cp = char_cast<char32_t>(cu & 0x03ff);
                cu = *it;
                if (cu >= 0xdc00 and cu < 0xe000) {
                    ++it;
                    cp <<= 10;
                    cp |= cu & 0x03ff;
                    cp += 0x01'0000;
                    return {cp, true};

                } else {
                    // unpaired surrogate.
                    return {0xfffd, false};
                }
            }

        } else if (cu < 0xe000) {
            // Invalid low surrogate.
            return {0xfffd, false};

        } else {
            return {cu, true};
        }
    }

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));
        return {truncate<uint8_t>((code_point >= 0x01'0000) + 1), true};
    }

    template<typename It>
    constexpr void write(char32_t code_point, It& dst) const noexcept
    {
        hi_axiom(code_point <= 0x10'ffff);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        if (hilet tmp = truncate<int32_t>(code_point) - 0x1'0000; tmp >= 0) {
            *dst++ = char_cast<char16_t>((tmp >> 10) + 0xd800);
            *dst++ = char_cast<char16_t>((tmp & 0x3ff) + 0xdc00);

        } else {
            *dst++ = char_cast<char16_t>(code_point);
        }
    }

#if defined(HI_HAS_SSE2)
    template<typename It>
    hi_force_inline __m128i read_ascii_chunk16(It it) const noexcept
    {
        // Load the UTF-16 data.
        hilet lo = _mm_loadu_si128(reinterpret_cast<__m128i const *>(std::addressof(*it)));
        it += 8;
        hilet hi = _mm_loadu_si128(reinterpret_cast<__m128i const *>(std::addressof(*it)));

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

    template<typename It>
    hi_force_inline void write_ascii_chunk16(__m128i chunk, It dst) const noexcept
    {
        hilet zero = _mm_setzero_si128();
        hilet lo = _mm_unpacklo_epi8(chunk, zero);
        hilet hi = _mm_unpackhi_epi8(chunk, zero);

        _mm_storeu_si128(reinterpret_cast<__m128i *>(std::addressof(*dst)), lo);
        dst += 8;
        _mm_storeu_si128(reinterpret_cast<__m128i *>(std::addressof(*dst)), hi);
    }
#endif
};

}} // namespace hi::v1

hi_warning_pop();
