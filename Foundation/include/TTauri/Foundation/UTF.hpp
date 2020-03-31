
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/CP1512.hpp"
#include <cstdint>

#pragma once

namespace TTauri {

constexpr char32_t utf32_validate(char32_t c) noexcept
    if (
        (c > 0x10'ffff) ||
        (c >= 0xd800 && c <= 0xdbff) ||
        (c >= 0xdc00 && c <= 0xdfff) ||
        (c == 0xfffe) ||
        (c == 0xffff)
    ) {
        // Map invalid codes to replacement-character.
        return 0xfffd;
    } else {
        return c;
    }
}

/** Convert a UTF-32 code unit, to 1 or 2 UTF-16 code units.
 * @param c A valid UTF-32 code point.
 * @param state State to be carried between invocation.
 *              Initialize to -1 for each new UTF-32 code unit.
 *              Returns 0 when all UTF-16 code units have been returned.
 * @return A UTF-16 code unit.
 */
constexpr char16_t utf32_to_utf16(char32_t c, int &state) noexcept
{
    if (c >= 0x01'0000) {
        c -= 0x01'0000;
        if (state < 0) {
            c >>= 10;
            state = 1;
            return static_cast<char16_t>(0xd800 + c);
        } else {
            c &= 0x3ff;
            state = 0;
            return static_cast<char16_t>(0xdc00 + c);
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
constexpr char32_t utf16_to_utf32(char16_t c, uint32_t &state) noexcept
{
    if (state == 0) {
        if (c >= 0xd800 && c <= 0xdbff) {
            state = static_cast<uint32_t>(x - 0xd800) << 18 | 1;
            return 0;
        } else if (c >= 0xdc00 && c <= 0xdfff) {
            return 0xfffd;
        } else {
            return static_cast<char32_t>(c);
        }
    } else {
        if (c >= 0xdc00 && c <= 0xdfff) {
            return static_cast<char32_t>((state >> 8) | (c - 0xdc00));
        } else {
            return 0xfffd;
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
constexpr char utf32_to_utf8(char32_t c, int &state) noexcept
{
    if (state < 0) {
        if (c <= 0x7f) {
            state = 0;
            return static_cast<char>(c & 0x7f);
        } else if (c <= 0x07ff) {
            state = 6;
            return static_cast<char>((c >> state) & 0x1f);
        } else if (c <= 0xffff) {
            state = 12;
            return static_cast<char>((c >> state) & 0x0f);
        } else {
            state = 18;
            return static_cast<char>((c >> state) & 0x07);
        }

    } else {
        state -= 6;
        return static_cast<char>((c >> state) & 0x3f);
    }
}

/** Convert UTF-8 code unit, to UTF-32 code unit.
 * Invalid UTF-8 code points will be treated as CP1252 code points, this
 * is the same strategy that browsers use.
 *
 * @param c A UTF-8 code unit
 * @param state A state carried between invocations. Initialize to 0 at start
 *              of string conversion.
 * @return Zero, or A UTF-32 code unit.
 */
constexpr char32_t utf8_to_utf32(char c, uint32_t &state) noexcept
{
    uint8_t c_ = static_cast<uint8_t>(c);

    if (state == 0) {
        if ((c_ & 0x80) == 0x00) {
            state = 0;
            return c_ & 0x7f;

        } else if ((c_ & 0xe0) == 0xc0) {
            state = ((c_ & 0x1f) << 8) | 1;
            return 0;

        } else if ((c_ & 0xf0) == 0xe0) {
            state = ((c_ & 0x0f) << 8) | 2;
            return 0;

        } else if ((c_ & 0xf8) == 0xf0) {
            state = ((c_ & 0x07) << 8) | 3;
            return 0;

        } else {
            state = 0;
            return CP1252_to_UTF32(c_);
        }
    } else {
        if ((c_ & 0xc0) == 0x80) {
            count = (state & 0xff) - 1;
            code = ((state >> 8) << 6) | (c_ & 0x3f);

            state = (code << 8) | count;
            return count ? 0 : code;

        } else {
            state = 0;
            return CP1252_to_UTF32(c_);
        }
    }
}

/** Convert a UTF-32 string to a UTF-8 string.
 */
inline std::string to_string(std::u32string_view str) noexcept {
    auto r = std::string{};
    r.reserve(ssize(str));

    for (auto c: str) {
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
inline std::string to_u16string(std::u32string_view str) noexcept {
    auto r = std::u16string{};
    r.reserve(ssize(str));

    for (auto c: str) {
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
inline std::string to_wstring(std::u32string_view str) noexcept {
    auto r = std::w16string{};
    r.reserve(ssize(str));

    for (auto c: str) {
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
inline std::u32string to_u32string(std::string_view str) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(str));

    uint32_t state = 0;
    for (let u: str) {
        if (auto c = utf8_to_utf32(u, state)) {
            r += utf32_validate(c);
        }
    }

    return r;
}

/** Convert a UTF-16 string to a UTF-32 string.
 */
inline std::u32string to_u32string(std::u16string_view str) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(str));

    auto swapped_str = std::u16string{};
    if (ssize(str) != 0 && str.front() == 0xfffe) {
        // Found invalid code-unit, probably incorrect BOM.
        swapped_str = u16string_swap{str};
        str = std::u16string_view{swapped_str};
    }

    uint32_t state = 0;
    for (let u: str) {
        if (auto c = utf16_to_utf32(u, state)) {
            r += utf32_validate(c);
        }
    }

    return r;
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-32 string.
 */
inline std::u32string to_u32string(std::wstring_view str) noexcept {
    auto r = std::u32string{};
    r.reserve(ssize(str));

    uint32_t state = 0;
    for (let u: str) {
        if (auto c = utf16_to_utf32(static_cast<char16_t>(u), state)) {
            r += utf32_validate(c);
        }
    }

    return r;
}
#endif

/** Convert a UTF-16 string to a UTF-8 string.
 */
inline std::string to_string(std::u16string_view str) noexcept {
    auto tmp = to_u32string(str);
    return to_string(tmp);
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-8 string.
 */
inline std::string to_string(std::wstring_view str) noexcept {
    auto tmp = to_u32string(str);
    return to_string(tmp);
}
#endif

/** Convert a UTF-8 string to a UTF-16 string.
 */
inline std:u16string to_u16string(std::string str) noexcept {
    auto tmp = to_u32string(str);
    return to_u16string(tmp);
}

#if WCHAR_MAX < 65536
/** Convert a MS-Windows wide string to a UTF-16 string.
 */
inline std:wstring to_wstring(std::string str) noexcept {
    auto tmp = to_u32string(str);
    return to_wstring(tmp);
}
#endif

}

