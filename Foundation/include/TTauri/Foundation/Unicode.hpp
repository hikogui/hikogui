// Copyright 2020 Pokitec
// All rights reserved.

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/endian.hpp"
#include "TTauri/Foundation/CP1252.hpp"
#include "TTauri/Foundation/math.hpp"
#include <cstdint>

#pragma once

namespace TTauri {

constexpr char32_t UnicodeASCIIEnd = 0x7f;
constexpr char32_t UnicodePlane0End = 0xffff;
constexpr char32_t UnicodePlane1Begin = 0x010000;
constexpr char32_t UnicodePlane16End = 0x10ffff;
constexpr char32_t UnicodeSurrogatesBegin = 0xd800;
constexpr char32_t UnicodeSurrogatesEnd = 0xdfff;
constexpr char32_t UnicodeHighSurrogatesBegin = 0xd800;
constexpr char32_t UnicodeHighSurrogatesEnd = 0xdbff;
constexpr char32_t UnicodeLowSurrogatesBegin = 0xdc00;
constexpr char32_t UnicodeLowSurrogatesEnd = 0xdfff;
constexpr char32_t UnicodeBasicMultilinqualPlaneEnd = UnicodePlane0End;
constexpr char32_t UnicodeMax = UnicodePlane16End;
constexpr char32_t UnicodeZeroWidthNoBreakSpace = 0xfeff;
constexpr char32_t UnicodeBOM = UnicodeZeroWidthNoBreakSpace;
constexpr char32_t UnicodeReplacementCharacter = 0xfffd;
constexpr char32_t UnicodeNonCharacterFFFE = 0xfffe;
constexpr char32_t UnicodeNonCharacterFFFF = 0xffff;
constexpr char32_t UnicodeReverseBOM = UnicodeNonCharacterFFFE;

[[nodiscard]] inline std::u32string splitLigature(char32_t x) noexcept
{
    switch (x) {
    case 0xfb00: return { 0x0066, 0x0066 }; // ff
    case 0xfb01: return { 0x0066, 0x0069 }; // fi
    case 0xfb02: return { 0x0066, 0x006c }; // fl
    case 0xfb03: return { 0x0066, 0x0066, 0x0069 }; // ffi
    case 0xfb04: return { 0x0066, 0x0066, 0x006c }; // ffl
    case 0xfb05: return { 0x017f, 0x0074 }; // long st
    case 0xfb06: return { 0x0073, 0x0074 }; // st

    case 0xfb13: return { 0x0574, 0x0576 }; // men now
    case 0xfb14: return { 0x0574, 0x0565 }; // men ech
    case 0xfb15: return { 0x0574, 0x056b }; // men ini
    case 0xfb16: return { 0x057e, 0x0576 }; // vew now
    case 0xfb17: return { 0x0574, 0x056d }; // men xeh

    default: return {};
    }
}

[[nodiscard]] constexpr char32_t utf32_validate(char32_t c) noexcept
{
    return (
        (c > UnicodeMax) ||
        (c >= UnicodeSurrogatesBegin && c <= UnicodeSurrogatesEnd) ||
        (c == UnicodeNonCharacterFFFE) ||
        (c == UnicodeNonCharacterFFFF)
    ) ?
        UnicodeReplacementCharacter :
        c;
}

template<typename UnaryOperation>
[[nodiscard]] inline std::u16string u16string_transform(std::u16string_view str, UnaryOperation unary_op) noexcept
{
    auto r = std::u16string{};
    r.reserve(ssize(str));

    std::transform(str.cbegin(), str.cend(), std::back_inserter(r), unary_op);
    return r;
}

[[nodiscard]] inline std::u16string u16string_byte_swap(std::u16string_view str) noexcept
{
    return u16string_transform(str, [](let &c) { return byte_swap(c); });
}

[[nodiscard]] inline std::u16string u16string_little_to_native(std::u16string_view str) noexcept
{
    return u16string_transform(str, [](let &c) { return little_to_native(c); });
}

[[nodiscard]] inline std::u16string u16string_big_to_native(std::u16string_view str) noexcept
{
    return u16string_transform(str, [](let &c) { return big_to_native(c); });
}

/** Convert a UTF-32 code unit, to 1 or 2 UTF-16 code units.
 * @param c A valid UTF-32 code point.
 * @param state State to be carried between invocation.
 *              Initialize to -1 for each new UTF-32 code unit.
 *              Returns 0 when all UTF-16 code units have been returned.
 * @return A UTF-16 code unit.
 */
[[nodiscard]] constexpr char16_t utf32_to_utf16(char32_t c, int &state) noexcept
{
    if (c >= UnicodePlane1Begin) {
        c -= UnicodePlane1Begin;
        if (state < 0) {
            c >>= 10;
            state = 1;
            return static_cast<char16_t>(UnicodeHighSurrogatesBegin + c);
        } else {
            c &= 0x3ff;
            state = 0;
            return static_cast<char16_t>(UnicodeLowSurrogatesBegin + c);
        }

    } else {
        state = 0;
        return static_cast<char16_t>(c);
    }
}

/** Convert UTF-16 code unit, to UTF-32 code unit.
 * Invalid/Unpaired surrogates will be dropped or converted to unknown-character.
 *
 * @param c A UTF-16 code unit
 * @param state A state carried between invocations. Initialize to 0 at start
 *              of string conversion.
 * @return Zero, or A UTF-32 code unit.
 */
[[nodiscard]] constexpr char32_t utf16_to_utf32(char16_t c, uint32_t &state) noexcept
{
    if (state == 0) {
        if (c >= UnicodeHighSurrogatesBegin && c <= UnicodeHighSurrogatesEnd) {
            state = static_cast<uint32_t>(c - UnicodeHighSurrogatesBegin) << 18 | 1;
            return 0;
        } else if (c >= UnicodeLowSurrogatesBegin && c <= UnicodeLowSurrogatesEnd) {
            return UnicodeReplacementCharacter;
        } else {
            return static_cast<char32_t>(c);
        }
    } else {
        if (c >= UnicodeLowSurrogatesBegin && c <= UnicodeLowSurrogatesEnd) {
            let upper10bits = static_cast<char32_t>(state >> 8);
            let lower10bits = static_cast<char32_t>(c - UnicodeLowSurrogatesBegin);
            state = 0;
            return (upper10bits | lower10bits) + UnicodePlane1Begin;
        } else {
            state = 0;
            return UnicodeReplacementCharacter;
        }
    }
}

/** Convert a UTF-32 code unit, to 1-4 UTF-8 code units.
 * @param c A valid UTF-32 code point.
 * @param state State to be carried between invocation.
 *              Initialize to -1 for each new UTF-32 code unit.
 *              Returns 0 when all UTF-8 code units have been returned.
 * @return A UTF-8 code unit.
 */
[[nodiscard]] constexpr char utf32_to_utf8(char32_t c, int &state) noexcept
{
    if (state < 0) {
        if (c <= 0x7f) {
            state = 0;
            return static_cast<char>(c);
        } else if (c <= 0x07ff) {
            state = 6;
            return static_cast<char>((c >> state) | 0xc0);
        } else if (c <= 0xffff) {
            state = 12;
            return static_cast<char>((c >> state) | 0xe0);
        } else {
            state = 18;
            return static_cast<char>((c >> state) | 0xf0);
        }

    } else {
        state -= 6;
        return static_cast<char>(((c >> state) & 0x3f) | 0x80);
    }
}

[[nodiscard]] no_inline inline char32_t utf8_to_utf32_fallback(char c) noexcept
{
    return CP1252_to_UTF32(c);
}

struct utf8_to_utf32_state {
    int trailing_bytes;
    char32_t code;

    utf8_to_utf32_state() noexcept : trailing_bytes(0) {}
};

/** Convert UTF-8 code unit, to UTF-32 code unit.
 * Invalid UTF-8 code points will be treated as CP1252 code points, this
 * is the same strategy that browsers use.
 *
 * @param c A UTF-8 code unit
 * @param state A state carried between invocations. Initialize to 0 at start
 *              of string conversion.
 * @return Zero, or A UTF-32 code unit.
 */
[[nodiscard]] constexpr char32_t utf8_to_utf32(char c, utf8_to_utf32_state &state) noexcept
{
    auto c_ = static_cast<uint8_t>(c);

    if (state.trailing_bytes) {
        if ((c_ & 0xc0) == 0x80) {
            --state.trailing_bytes;
            state.code <<= 6;
            state.code |= (c_ & 0x3f);
            return state.trailing_bytes ? 0 : state.code;

        } else {
            state.trailing_bytes = 0;
            return utf8_to_utf32_fallback(c_);
        }

    } else {
        let inv_c32 = static_cast<uint32_t>(static_cast<uint8_t>(~c_));
        let nr_data_bits = bsr(inv_c32);

        state.trailing_bytes = 6 - nr_data_bits;
        if (state.trailing_bytes < 0) {
            // 0b0xxxxxxx
            state.trailing_bytes = 0;
            return c_ & 0x7f;

        } else if (state.trailing_bytes > 0 && state.trailing_bytes <= 3) {
            // 0b110xxxxx, 0b1110xxxx, 0b11110xxx,
            let data_mask = (1 << nr_data_bits) - 1;
            state.code = (c_ & data_mask);
            return 0;

        } else {
            // 0b10xxxxx
            // 0b111110xx, 0b1111110x, 0b11111110
            state.trailing_bytes = 0;
            return utf8_to_utf32_fallback(c_);
        }   
    }   
}

/** Convert a UTF-32 string to a UTF-8 string.
 */
[[nodiscard]] inline std::string to_string(std::u32string_view rhs) noexcept {
    auto r = std::string{};
    r.reserve(ssize(rhs));

    for (auto c: rhs) {
        c = utf32_validate(c);

        int state = -1;
        do {    
            r += utf32_to_utf8(c, state);
        } while (state);
    }

    return r;
}

/** Convert a UTF-32 string to a UTF-16 string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::u32string_view rhs) noexcept {
    auto r = std::u16string{};
    r.reserve(ssize(rhs));

    for (auto c: rhs) {
        c = utf32_validate(c);

        int state = -1;
        do {    
            r += utf32_to_utf16(c, state);
        } while (state);
    }

    return r;
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-16 string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::u32string_view rhs) noexcept {
    auto r = std::wstring{};
    r.reserve(ssize(rhs));

    for (auto c: rhs) {
        c = utf32_validate(c);

        int state = -1;
        do {    
            r += static_cast<wchar_t>(utf32_to_utf16(c, state));
        } while (state);
    }

    return r;
}
#endif

/** Convert a UTF-8 string to a UTF-32 string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::string_view rhs) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(rhs));

    auto state = utf8_to_utf32_state{};
    for (let u: rhs) {
        if (auto c = utf8_to_utf32(u, state)) {
            r += utf32_validate(c);
        }
    }

    return r;
}

/** Convert a UTF-16 string to a UTF-32 string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::u16string_view rhs) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(rhs));

    auto swapped_str = std::u16string{};
    if (ssize(rhs) != 0 && rhs.front() == UnicodeReverseBOM) {
        swapped_str = u16string_byte_swap(rhs);
        rhs = std::u16string_view{swapped_str};
    }

    uint32_t state = 0;
    for (let u: rhs) {
        if (auto c = utf16_to_utf32(u, state)) {
            ttauri_assume(c <= UnicodeMax);
            r += utf32_validate(c);
        }
    }

    return r;
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-32 string.
 */
[[nodiscard]] inline std::u32string to_u32string(std::wstring_view rhs) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(rhs));

    uint32_t state = 0;
    for (let u: rhs) {
        if (auto c = utf16_to_utf32(static_cast<char16_t>(u), state)) {
            r += utf32_validate(c);
        }
    }

    return r;
}
#endif

/** Convert a UTF-16 string to a UTF-8 string.
 */
[[nodiscard]] inline std::string to_string(std::u16string_view rhs) noexcept {
    return to_string(to_u32string(rhs));
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-8 string.
 */
[[nodiscard]] inline std::string to_string(std::wstring_view rhs) noexcept {
    return to_string(to_u32string(rhs));
}
#endif

/** Convert a UTF-8 string to a UTF-16 string.
 */
[[nodiscard]] inline std::u16string to_u16string(std::string_view rhs) noexcept {
    return to_u16string(to_u32string(rhs));
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-16 string.
 */
[[nodiscard]] inline std::wstring to_wstring(std::string_view rhs) noexcept {
    return to_wstring(to_u32string(rhs));
}
#endif

}

