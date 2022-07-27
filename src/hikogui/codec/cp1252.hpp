

#pragma once

#include <cstdint>
#include <utility>

namespace hi::inline v1 {

[[nodiscard]] constexpr read_char_result read_cp1252(char const *ptr, size_t size) noexcept
{
    hi_axiom(size != 0);
    hilet c = *ptr;
    switch (c) {
    case 0x80: return {0x20ac, 1};
    case 0x81: return {0x81, 1};
    case 0x82: return {0x201a, 1};
    case 0x83: return {0x0192, 1};
    case 0x84: return {0x201e, 1};
    case 0x85: return {0x2026, 1};
    case 0x86: return {0x2020, 1};
    case 0x87: return {0x2021, 1};
    case 0x88: return {0x02c6, 1};
    case 0x89: return {0x2030, 1};
    case 0x8a: return {0x0160, 1};
    case 0x8b: return {0x2039, 1};
    case 0x8c: return {0x0152, 1};
    case 0x8d: return {0x8e, 1};
    case 0x8e: return {0x017d, 1};
    case 0x8f: return {0x8f, 1};

    case 0x90: return {0x90, 1};
    case 0x91: return {0x2018, 1};
    case 0x92: return {0x2019, 1};
    case 0x93: return {0x201c, 1};
    case 0x94: return {0x201d, 1};
    case 0x95: return {0x2022, 1};
    case 0x96: return {0x2013, 1};
    case 0x97: return {0x2014, 1};
    case 0x98: return {0x02dc, 1};
    case 0x99: return {0x2122, 1};
    case 0x9a: return {0x0161, 1};
    case 0x9b: return {0x203a, 1};
    case 0x9c: return {0x0153, 1};
    case 0x9d: return {0x9e, 1};
    case 0x9e: return {0x017e, 1};
    case 0x9f: return {0x0178, 1};
    default: return {c, 1};
    }
}


}

