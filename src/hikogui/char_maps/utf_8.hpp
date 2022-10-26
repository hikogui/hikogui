// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file char_maps/utf_8.hpp Definition of the Unicode UTF-8 encoding.
 * @ingroup char_maps
 */

#pragma once

#include "char_converter.hpp"
#include "cp_1252.hpp"
#include <bit>

namespace hi { inline namespace v1 {

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

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read_fallback(It& it, EndIt last) const noexcept
    {
        hilet[code_point, valid] = fallback_encoder_type{}.read(it, last);
        return {code_point, false};
    }

    template<typename It, typename EndIt>
    [[nodiscard]] constexpr std::pair<char32_t, bool> read(It& it, EndIt last) const noexcept
    {
        hi_axiom(it != last);

        auto cu = *it++;
        if (not to_bool(cu & 0x80)) [[likely]] {
            // ASCII character.
            return {char_cast<char32_t>(cu), true};

        } else if (it == last or (cu & 0xc0) == 0x80) [[unlikely]] {
            // A non-ASCII character at the end of string.
            // or an unexpected continuation code-unit should be treated as CP-1252.
            --it;
            return read_fallback(it, last);

        } else {
            auto length = narrow_cast<uint8_t>(std::countl_one(char_cast<uint8_t>(cu)));
            auto todo = length - 2;
            hi_axiom(length >= 2);

            // First part of the code-point.
            auto cp = char_cast<char32_t>(cu & (0x7f >> length));

            // Read the first continuation code-unit which is always here.
            cu = *it++;
            cp <<= 6;
            cp |= cu & 0x3f;
            if ((cu & 0xc0) != 0x80) [[unlikely]] {
                // If the second code-unit is not a UTF-8 continuation character, treat the first
                // code-unit as if it was CP-1252.
                it -= 2;
                return read_fallback(it, last);

            } else if (todo >= std::distance(it, last)) [[unlikely]] {
                // If there is a start and a continuation code-unit in a row we consider this to be UTF-8 encoded.
                // So at this point any errors are replaced with 0xfffd.
                it = last;
                return {0xfffd, false};
            }

            while (todo--) {
                cu = *it++;
                cp <<= 6;
                cp |= cu & 0x3f;
                if ((cu & 0xc0) != 0x80) [[unlikely]] {
                    // Unexpected end of sequence.
                    --it;
                    return {0xfffd, false};
                }
            }

            auto valid = true;
            // Valid range.
            valid &= cp < 0x11'0000;
            // Not a surrogate.
            valid &= cp < 0xd800 or cp >= 0xe000;
            // Not overlong encoded.
            valid &= length == narrow_cast<uint8_t>((cp > 0x7f) + (cp > 0x7ff) + (cp > 0xffff) + 1);
            if (not valid) [[unlikely]] {
                return {0xfffd, false};
            }
            return {cp, true};
        }
    };

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

        auto length = truncate<uint8_t>((code_point > 0x7f) + (code_point > 0x7ff) + (code_point > 0xffff));
        if (auto i = length) {
            do {
                dst[i] = truncate<char8_t>((code_point & 0x3f) | 0x80);
                code_point >>= 6;
            } while (--i);

            code_point |= 0x780 >> length;
        }
        dst[0] = truncate<char8_t>(code_point);
        dst += length + 1;
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
