// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../assert.hpp"
#include "../cast.hpp"
#include <cstdint>

namespace hi::inline v1 {



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
