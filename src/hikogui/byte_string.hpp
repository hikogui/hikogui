// Copyright Take Vos 2019, 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "hash.hpp"
#include "cast.hpp"
#include <cstddef>
#include <string>
#include <string_view>
#include <cstring>
#include <concepts>
#include <type_traits>

hi_warning_push();
// C26490: Don't use reinterpret_cast (type.1).
// Need to call strlen() and friends with a `char *`.
hi_warning_ignore_msvc(26490);

namespace hi::inline v1 {

using byteptr = std::byte *;
using cbyteptr = std::byte const *;

class byte_char_traits {
public:
    using char_type = std::byte;
    using int_type = unsigned int;
    using off_type = std::streamoff;
    using pos_type = std::fpos<std::mbstate_t>;
    using state_type = std::mbstate_t;

    static constexpr void assign(std::byte& r, std::byte const& a) noexcept
    {
        r = a;
    }

    static constexpr bool eq(std::byte a, std::byte b) noexcept
    {
        return static_cast<uint8_t>(a) == static_cast<uint8_t>(b);
    }

    static constexpr bool lt(std::byte a, std::byte b) noexcept
    {
        return static_cast<uint8_t>(a) < static_cast<uint8_t>(b);
    }

    static std::byte *assign(std::byte *p, std::size_t count, char_type a) noexcept
    {
        return static_cast<std::byte *>(std::memset(p, static_cast<uint8_t>(a), count));
    }

    static std::byte *move(std::byte *dest, std::byte const *src, std::size_t count) noexcept
    {
        return static_cast<std::byte *>(std::memmove(dest, src, count));
    }

    static std::byte *copy(std::byte *dest, std::byte const *src, std::size_t count) noexcept
    {
        return static_cast<std::byte *>(std::memcpy(dest, src, count));
    }

    static int compare(std::byte const *a, std::byte const *b, std::size_t count) noexcept
    {
        return std::memcmp(a, b, count);
    }

    static std::size_t length(std::byte const *s) noexcept
    {
        return std::strlen(reinterpret_cast<char const *>(s));
    }

    static std::byte const *find(std::byte const *s, std::size_t count, std::byte const& ch) noexcept
    {
        return static_cast<std::byte const *>(std::memchr(reinterpret_cast<char const *>(s), static_cast<uint8_t>(ch), count));
    }

    static constexpr std::byte to_char_type(unsigned int c) noexcept
    {
        return static_cast<std::byte>(c);
    }

    static constexpr unsigned int to_int_type(std::byte c) noexcept
    {
        return static_cast<unsigned int>(c);
    }

    static constexpr bool eq_int_type(unsigned int c1, unsigned int c2) noexcept
    {
        return c1 == c2;
    }

    static constexpr unsigned int eof() noexcept
    {
        return 256;
    }

    static constexpr unsigned int not_eof(unsigned int e) noexcept
    {
        return eq_int_type(e, eof()) ? 0 : e;
    }
};

using bstring = std::basic_string<std::byte, byte_char_traits>;
using bstring_view = std::basic_string_view<std::byte, byte_char_traits>;

[[nodiscard]] inline bstring to_bstring(std::string_view src) noexcept
{
    return bstring{reinterpret_cast<std::byte const *>(src.data()), src.size()};
}

[[nodiscard]] inline bstring to_bstring(std::integral auto... args) noexcept
{
    return bstring{{static_cast<std::byte>(args)...}};
}

} // namespace hi::inline v1

template<>
struct std::hash<hi::bstring> {
    [[nodiscard]] size_t operator()(hi::bstring const& rhs) const noexcept
    {
        auto r = size_t{0};
        for (auto c : rhs) {
            r = hi::hash_mix_two(r, std::hash<uint8_t>{}(hi::char_cast<uint8_t>(c)));
        }
        return r;
    }
};

template<>
struct std::hash<hi::bstring_view> {
    [[nodiscard]] size_t operator()(hi::bstring_view const& rhs) const noexcept
    {
        auto r = size_t{0};
        for (auto c : rhs) {
            r = hi::hash_mix_two(r, std::hash<uint8_t>{}(hi::char_cast<uint8_t>(c)));
        }
        return r;
    }
};

hi_warning_pop();
