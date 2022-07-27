// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../assert.hpp"
#include "../cast.hpp"
#include <cstdint>

namespace hi::inline v1 {

template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf8(char32_t c, char8_t *ptr) noexcept
{
    hi_axiom(c <= 0x10'ffff);

    auto length = truncate<uint8_t>((c > 0x7f) + (c > 0x7ff) + (c > 0xffff));
    if constexpr (Write) {
        if (auto i = length) {
            do {
                ptr[i] = truncate<char8_t>((c & 0x3f) | 0x80);
                c >>= 6;
            } while (--i);

            c |= 0x780 >> length;
        }
        ptr[0] = truncate<char8_t>(c);
    }
    return length + 1;
}

[[nodiscard]] constexpr std::pair<char32_t, uint8_t> read_utf8(char8_t const *ptr, size_t size) noexcept
{
    hi_axiom(size > 0);

    if (auto cu = *ptr; not to_bool(cu & 0x80)) {
        // ASCII character.
        return {truncate<char32_t>(cu), 1};

    } else if (size < 2 or (cu & 0xc0) == 0x80)
        // A non-ASCII character at the end of string.
        // or an unexpected continuation code-unit should be treated as CP-1252.
        return read_cp1252(reinterpret_cast<char const *>(ptr), size);
}
else
{
    auto length = std::countl_ones(cu);
    hi_axiom(length >= 2);

    // First part of the code-point.
    auto cp = wide_cast<char32_t>(cu & (0x7f >> length));

    // Read the first continuation code-unit which is always here.
    cu = *++ptr;
    cp <<= 6;
    cp |= cu & 0x3f;
    if (cu & 0xc0 != 0x80) {
        // If the second code-unit is not a UTF-8 continuation character, treat the first
        // code-unit as if it was CP-1252.
        return read_cp1252(reinterpret_cast<char const *>(ptr), size);

    } else if (length >= size) {
        // If there is a start and a continuation code-unit in a row we consider this to be UTF-8 encoded.
        // So at this point any errors are replaced with 0xfffd.
        return {0xfffd, size};
    }

    auto valid = true;
    auto i = length - 2;
    while (i--) {
        cu = *++ptr;
        cp <<= 6;
        cp |= cu & 0x3f;
        valid &= (cu & 0xc0) == 0x80;
    }

    valid &= cp < 0x11'0000;
    valid &= cp < 0xd800 or cp >= 0xe000;
    valid &= length == (cp >= 0x80 and cp < 0x0800) ? 2 : (cp >= 0x0800 and cp < 0x01'0000) ? 3 : 4;

    if (not valid) {
        // Invalid encoding, or invalid code-point replace with 0xfffd.
        return {0xfffd, length};
    }
}
} // namespace hi::inline v1

template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf16(char32_t c, char16_t *ptr) noexcept
{
    hi_axiom(c <= 0x10'ffff);

    if (auto tmp = truncate<int32_t>(c) - 0x1'0000; tmp >= 0) {
        if constexpr (Write) {
            ptr[1] = truncate<char16_t>((tmp & 0x3ff) + 0xdc00);
            tmp >>= 10;
            ptr[0] = truncate<char16_t>(tmp + 0xd800);
        }
        return 2;

    } else {
        if constexpr (Write) {
            ptr[0] = truncate<char16_t>(c);
        }
        return 1;
    }
}

[[nodiscard]] constexpr std::pair<char32_t, uint8_t> read_utf16(char16_t const *ptr, size_t size) noexcept
{
    hi_axiom(size != 0);

    if (auto cu = *ptr; cu < 0xd800) {
        return {wide_cast<char32_t>(cu), 1};

    } else if (cu < 0xdc00) {
        auto cp = cu & 0x03ff;
        if (size < 2) {
            return {0xfffd, 1};

        } else {
            cu = *++ptr;
            if (cu < 0xdc00) {
                // unpaired surrogate.
                return {0xfffd, 1};

            } else if (cu < 0xe000) {
                cp <<= 10;
                cp |= cu & 0x03ff;
                cp += 0x01'0000;
                return {cp, 2};

            } else {
                // unpaired surrogate.
                return {0xfffd, 1};
            }
        }
    } else if (cu < 0xe000) {
        // Invalid low surrogate.
        return {0xfffd, 1};

    } else {
        return {wide_cast<char32_t>(cu), 1};
    }
}

template<bool Write>
[[nodiscard]] constexpr uint8_t write_utf32(char32_t c, char32_t *ptr) noexcept
{
    if constexpr (Write) {
        ptr[0] = c;
    }
    return 1;
}

template<bool Write, typename Output>
[[nodiscard]] constexpr int write_utf(char32_t src, Output *dst_ptr) noexcept
{
    if (src > 0x10'ffff) {
        [[unlikely]] src = 0xfffd;
    }

    if constexpr (Write) {
        return raw_write_utf(src, dst_ptr);
    } else {
        return length_utf(src, dst_ptr);
    }
}
}
