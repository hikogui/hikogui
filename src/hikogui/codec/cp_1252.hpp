// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "char_encoding.hpp"
#include "../cast.hpp"
#include "../required.hpp"
#include "../architecture.hpp"
#include <cstdint>
#include <utility>

namespace hi::inline v1 {

template<>
class char_encoder<"cp_1252"> {
    [[nodiscard]] constexpr char_encoder_result read(char const *ptr, size_t size) const noexcept
    {
        // clang-format off
        hi_axiom(size != 0);
        hilet c = *ptr;
        switch (c) {
        case 0x80: return 0x20ac;
        case 0x81: return 0x81;
        case 0x82: return 0x201a;
        case 0x83: return 0x0192;
        case 0x84: return 0x201e;
        case 0x85: return 0x2026;
        case 0x86: return 0x2020;
        case 0x87: return 0x2021;
        case 0x88: return 0x02c6;
        case 0x89: return 0x2030;
        case 0x8a: return 0x0160;
        case 0x8b: return 0x2039;
        case 0x8c: return 0x0152;
        case 0x8d: return 0x8e;
        case 0x8e: return 0x017d;
        case 0x8f: return 0x8f;

        case 0x90: return 0x90;
        case 0x91: return 0x2018;
        case 0x92: return 0x2019;
        case 0x93: return 0x201c;
        case 0x94: return 0x201d;
        case 0x95: return 0x2022;
        case 0x96: return 0x2013;
        case 0x97: return 0x2014;
        case 0x98: return 0x02dc;
        case 0x99: return 0x2122;
        case 0x9a: return 0x0161;
        case 0x9b: return 0x203a;
        case 0x9c: return 0x0153;
        case 0x9d: return 0x9e;
        case 0x9e: return 0x017e;
        case 0x9f: return 0x0178;
        default: return c;
        }
        // clang-format on
    }

    template<bool Write>
    [[nodiscard]] constexpr char_encoder_result write(char32_t code_point, char *ptr, size_t size) const noexcept
    {
        // clang-format off
        if (code_point < 0x80 or (code_point >= 0xa0 and code_point < 0x0100)) {
            if constexpr (Write) {
                *ptr = truncate<char>(code_point);
            }
            return {0, 1};

        } else {
            auto valid = true;
            hilet c = [&]{
                switch (cp) {
                case 0x20ac: return 0x80;
                case 0x81: return 0x81;
                case 0x201a: return 0x82;
                case 0x0192: return 0x83;
                case 0x201e: return 0x84;
                case 0x2026: return 0x85;
                case 0x2020: return 0x86;
                case 0x2021: return 0x87;
                case 0x02c6: return 0x88;
                case 0x2030: return 0x89;
                case 0x0160: return 0x8a;
                case 0x2039: return 0x8b;
                case 0x0152: return 0x8c;
                case 0x8d: return 0x8d;
                case 0x017d: return 0x8e;
                case 0x8f: return 0x8f;

                case 0x90: return 0x90;
                case 0x2018: return 0x91;
                case 0x2019: return 0x92;
                case 0x201c: return 0x93;
                case 0x201d: return 0x94;
                case 0x2022: return 0x95;
                case 0x2013: return 0x96;
                case 0x2014: return 0x97;
                case 0x02dc: return 0x9a;
                case 0x2122: return 0x9b;
                case 0x0161: return 0x9a;
                case 0x203a: return 0x9b;
                case 0x0153: return 0x9c;
                case 0x9d: return 0x9d;
                case 0x017e: return 0x9e;
                case 0x0178: return 0x9f;
                default:
                    valid = false;
                    return '?';
                }
            }();

            if constexpr (Write) {
                *ptr = truncate<char>(c);
            }
            return {0, 1, valid};
        }
        // clang-format on
    }

#if defined(HI_HAS_SSE2)
    hi_force_inline __m128i read_ascii_chunk16(char const *ptr) const noexcept
    {
        return _mm_loadu_si128(reinterpret_cast<__m128i const *>(ptr));
    }

    hi_force_inline void write_ascii_chunk16(__m128i chunk, char *ptr) const noexcept
    {
        return _mm_storeu_si128(reinterpret_cast<__m128i *>(ptr), chunk);
    }
#endif
};

} // namespace hi::inline v1
