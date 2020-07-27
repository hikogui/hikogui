// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include "encoding/base16.hpp"

namespace tt {

using byteptr = std::byte *;
using cbyteptr = std::byte const *;

class byte_char_traits {
public:
    using char_type = std::byte;
    using int_type = unsigned int;
    using off_type = std::streamoff;
    using pos_type = std::fpos<std::mbstate_t>;
    using state_type = std::mbstate_t;

    static constexpr void assign(std::byte &r, std::byte const &a) noexcept {
        r = a;
    }

    static constexpr bool eq(std::byte a, std::byte b) noexcept {
        return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);
    }

    static constexpr bool lt(std::byte a, std::byte b) noexcept {
        return static_cast<uint8_t>(a) < static_cast<uint8_t>(b);
    }

    static std::byte *assign(std::byte *p, std::size_t count, char_type a) {
        return reinterpret_cast<std::byte *>(std::memset(p, static_cast<uint8_t>(a), count));
    }

    static std::byte *move(std::byte *dest, std::byte const *src, size_t count) {
        return reinterpret_cast<std::byte *>(std::memmove(dest, src, count));
    }

    static std::byte *copy(std::byte *dest, std::byte const *src, size_t count) {
        return reinterpret_cast<std::byte *>(std::memcpy(dest, src, count));
    }

    static int compare(std::byte const *a, std::byte const *b, size_t count) {
        return std::memcmp(a, b, count);
    }

    static size_t length(std::byte const *s) {
        return std::strlen(reinterpret_cast<char const *>(s));
    }

    static std::byte const *find(std::byte const *s, size_t count, std::byte const &ch) {
        return reinterpret_cast<std::byte const *>(std::memchr(reinterpret_cast<char const *>(s), static_cast<uint8_t>(ch), count));
    }

    static constexpr std::byte to_char_type(unsigned int c) noexcept {
        return static_cast<std::byte>(c);
    }

    static constexpr unsigned int to_int_type(std::byte c) noexcept {
        return static_cast<unsigned int>(c);
    }

    static constexpr bool eq_int_type(unsigned int c1, unsigned int c2) noexcept {
        return c1 == c2;
    }

    static constexpr unsigned int eof() noexcept {
        return 256;
    }

    static constexpr unsigned int not_eof(unsigned int e) noexcept {
        return eq_int_type(e, eof()) ? 0 : e;
    }
};

using bstring = std::basic_string<std::byte, byte_char_traits>;
using bstring_view = std::basic_string_view<std::byte, byte_char_traits>;


[[nodiscard]] inline bstring to_bstring(std::string src) noexcept {
    return bstring{reinterpret_cast<std::byte *>(src.data()), src.size()};
}

[[nodiscard]] inline std::string encode_base16(bstring const &str) noexcept
{
    ttlet first = str.data();
    ttlet last = first + str.size();
    return encode_base16(first, last);
}

[[nodiscard]] inline std::string encode_base16(bstring_view str) noexcept
{
    ttlet first = str.data();
    ttlet last = first + str.size();
    return encode_base16(first, last);
}

[[nodiscard]] inline std::string to_pretty_string(bstring const &src) noexcept
{
    return encode_base16(src);
}

}
