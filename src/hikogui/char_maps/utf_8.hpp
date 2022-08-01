// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_converter.hpp"
#include "cp_1252.hpp"
#include <bit>

namespace hi::inline v1 {

template<>
struct char_map<"utf-8"> {
    using char_type = char8_t;
    using fallback_encoder_type = char_map<"cp-1252">;
    using fallback_char_type = fallback_encoder_type::char_type;

    [[nodiscard]] constexpr std::pair<char32_t, bool> read_fallback(char_type const *& ptr, char_type const *last) const noexcept
    {
        hilet[code_point, valid] = fallback_encoder_type{}.read(
            reinterpret_cast<fallback_char_type const *&>(ptr), reinterpret_cast<fallback_char_type const *>(last));
        return {code_point, false};
    }

    [[nodiscard]] constexpr std::pair<char32_t, bool> read(char_type const *& ptr, char_type const *last) const noexcept
    {
        hi_axiom(ptr != last);

        auto cu = *ptr++;
        if (not to_bool(cu & 0x80)) [[likely]] {
            // ASCII character.
            return {char_cast<char32_t>(cu), true};

        } else if (ptr == last or (cu & 0xc0) == 0x80) [[unlikely]] {
            // A non-ASCII character at the end of string.
            // or an unexpected continuation code-unit should be treated as CP-1252.
            --ptr;
            return read_fallback(ptr, last);

        } else {
            auto length = narrow_cast<uint8_t>(std::countl_one(char_cast<uint8_t>(cu)));
            auto todo = length - 2;
            hi_axiom(length >= 2);

            // First part of the code-point.
            auto cp = char_cast<char32_t>(cu & (0x7f >> length));

            // Read the first continuation code-unit which is always here.
            cu = *ptr++;
            cp <<= 6;
            cp |= cu & 0x3f;
            if ((cu & 0xc0) != 0x80) [[unlikely]] {
                // If the second code-unit is not a UTF-8 continuation character, treat the first
                // code-unit as if it was CP-1252.
                ptr -= 2;
                return read_fallback(ptr, last);

            } else if (todo >= std::distance(ptr, last)) [[unlikely]] {
                // If there is a start and a continuation code-unit in a row we consider this to be UTF-8 encoded.
                // So at this point any errors are replaced with 0xfffd.
                ptr = last;
                return {0xfffd, false};
            }

            while (todo--) {
                cu = *ptr++;
                cp <<= 6;
                cp |= cu & 0x3f;
                if ((cu & 0xc0) != 0x80) [[unlikely]] {
                    // Unexpected end of sequence.
                    --ptr;
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

    constexpr void write(char32_t code_point, char_type *& ptr) const noexcept
    {
        hi_axiom(code_point < 0x11'0000);
        hi_axiom(not(code_point >= 0xd800 and code_point < 0xe000));

        auto length = truncate<uint8_t>((code_point > 0x7f) + (code_point > 0x7ff) + (code_point > 0xffff));
        if (auto i = length) {
            do {
                ptr[i] = truncate<char8_t>((code_point & 0x3f) | 0x80);
                code_point >>= 6;
            } while (--i);

            code_point |= 0x780 >> length;
        }
        ptr[0] = truncate<char8_t>(code_point);
        ptr += length + 1;
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char_type const *ptr) const noexcept
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
    }

    hi_force_inline void write_ascii_chunk16(__m128i chunk, char_type *ptr) const noexcept
    {
        _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), chunk);
    }
#endif
};

} // namespace hi::inline v1
