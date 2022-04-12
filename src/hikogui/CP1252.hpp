// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

namespace hi::inline v1 {

[[nodiscard]] constexpr char32_t CP1252_to_UTF32(char inputCharacter) noexcept
{
    hilet inputCharacter_ = static_cast<uint8_t>(inputCharacter);
    if (inputCharacter_ <= 0x7f) {
        return inputCharacter_;
    } else if (inputCharacter_ >= 0xa0) {
        return inputCharacter_;
    } else {
        switch (inputCharacter_) {
        case 0x80: return 0x20ac;
        case 0x81: return 0xfffd; // Replacement character.
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
        case 0x8d: return 0xfffd; // Replacement character.
        case 0x8e: return 0x017d;
        case 0x8f: return 0xfffd; // Replacement character.
        case 0x90: return 0xfffd; // Replacement character.
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
        case 0x9d: return 0xfffd; // Replacement character.
        case 0x9e: return 0x017e;
        case 0x9f: return 0x0178;
        default: return 0xfffd; // Replacement character.
        }
    }
}

} // namespace hi::inline v1
