// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/utf_8.hpp Definition of the Unicode UTF-8 encoding.
 * @ingroup char_maps
 */

module;
#include "../macros.hpp"

#include <bit>
#include <utility>
#include <cstddef>
#include <compare>
#if defined(HI_HAS_SSE2)
#include <emmintrin.h>
#endif

export module hikogui_char_maps_utf_8;
import hikogui_char_maps_char_converter;
import hikogui_char_maps_cp_1252;
import hikogui_utility;

hi_warning_push();
// C26490: Don't use reinterpret_cast.
// Needed for SIMD intrinsics.
hi_warning_ignore_msvc(26490);

export namespace hi { inline namespace v1 {

/** Unicode UTF-8 encoding.
 * @ingroup char_maps
 */
template<>
struct char_map<"utf-8"> {
    using char_type = char;
    using fallback_encoder_type = char_map<"cp-1252">;
    using fallback_char_type = fallback_encoder_type::char_type;

    [[nodiscard]] constexpr std::endian guess_endian(void const *ptr, size_t size, std::endian endian) const noexcept
    {
        return std::endian::native;
    }

    [[nodiscard]] constexpr std::pair<char32_t, bool> read_fallback(char const cu) const noexcept
    {
        hilet str = std::string_view(&cu, 1_uz);
        auto first = str.begin();
        hilet[code_point, valid] = fallback_encoder_type{}.read(first, str.end());
        return {code_point, false};
    }

    template<typename It, typename EndIt>
    [[nodiscard]] hi_no_inline constexpr std::pair<char32_t, bool> read(It& it, EndIt last, char8_t first_cu) const noexcept
    {
        if (it == last or (first_cu & 0xc0) == 0x80) {
            // A non-ASCII character at the end of string.
            // or an unexpected continuation code-unit should be treated as CP-1252.
            return read_fallback(char_cast<char>(first_cu));

        } else {
            hilet length = narrow_cast<uint8_t>(std::countl_one(char_cast<uint8_t>(first_cu)));
            hi_axiom(length >= 2);

            // First part of the code-point.
            auto cu = first_cu;
            cu <<= length;
            cu >>= length;
            auto cp = char_cast<char32_t>(cu);

            // Read the first continuation code-unit which is always here.
            cu = char_cast<char8_t>(*it);

            cp <<= 6;
            cp |= cu & 0x3f;
            if ((cu & 0xc0) != 0x80) {
                // If the second code-unit is not a UTF-8 continuation character, treat the first
                // code-unit as if it was CP-1252.
                return read_fallback(char_cast<char>(first_cu));
            }

            // If there are a start and a continuation code-unit in a sequence we consider this to be properly UTF-8 encoded.
            // So from this point any errors are replaced with 0xfffd.
            ++it;

            for (uint8_t actual_length = 2; actual_length != length; ++actual_length) {
                if (it == last) {
                    // End-of-file
                    return {0xfffd, false};
                }

                cu = char_cast<char8_t>(*it);
                if ((cu & 0b11'000000) != 0b10'000000) {
                    // Unexpected end of sequence.
                    return {0xfffd, false};
                }

                ++it;

                // Shift in the next 6 bits.
                cp <<= 6;
                cp |= cu & 0b00'111111;
            }

            auto valid = true;
            // Valid range.
            valid &= cp < 0x11'0000;
            // Not a surrogate.
            valid &= cp < 0xd800 or cp >= 0xe000;
            // Not overlong encoded.
            valid &= length == narrow_cast<uint8_t>((cp > 0x7f) + (cp > 0x7ff) + (cp > 0xffff) + 1);
            if (not valid) {
                return {0xfffd, false};
            }
            return {cp, true};
        }
    }

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read(It& it, EndIt last) const noexcept
    {
        hi_axiom(it != last);

        auto cu = char_cast<char8_t>(*it);
        ++it;
        if (not to_bool(cu & 0x80)) [[likely]] {
            // ASCII character.
            return {char_cast<char32_t>(cu), true};

        } else {
            return read(it, last, cu);
        }
    }

    [[nodiscard]] constexpr std::pair<uint8_t, bool> size(char32_t code_point) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        return {narrow_cast<uint8_t>((code_point > 0x7f) + (code_point > 0x7ff) + (code_point > 0xffff) + 1), true};
    }

    template<typename It>
    constexpr void write(char32_t code_point, It& dst) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        hilet num_cu = truncate<uint8_t>((code_point > 0x7f) + (code_point > 0x7ff) + (code_point > 0xffff));

        auto leading_ones = char_cast<int8_t>(uint8_t{0x80});
        leading_ones >>= num_cu;
        if (num_cu == 0) {
            leading_ones = 0;
        }

        auto shift = num_cu * 6;

        auto cu = truncate<uint8_t>(code_point >> shift);
        cu |= truncate<uint8_t>(leading_ones);

        // We can't cast `cu` to `dst` since it is not possible to get the
        // value_type of an output-iterator, specifically the
        // std::back_insert_iterator.
        *dst = cu;
        ++dst;

        while (shift) {
            shift -= 6;

            cu = truncate<uint8_t>(code_point >> shift);
            cu &= 0b00'111111;
            cu |= 0b10'000000;

            // We can't cast `cu` to `dst` since it is not possible to get the
            // value_type of an output-iterator, specifically the
            // std::back_insert_iterator.
            *dst = cu;
            ++dst;
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

hi_warning_pop();
