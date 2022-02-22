// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "grapheme.hpp"
#include "../required.hpp"
#include "../strings.hpp"
#include "../hash.hpp"
#include <vector>
#include <string>

template<>
struct std::char_traits<tt::grapheme> {
    using char_type = tt::grapheme;
    using int_type = tt::grapheme::value_type;
    using off_type = std::streamoff;
    using state_type = std::mbstate_t;
    using pos_type = std::fpos<state_type>;
    using comparison_category = std::strong_ordering;

    static constexpr void assign(char_type &r, char_type const &a) noexcept
    {
        r = a;
    }

    static constexpr char_type *assign(char_type *p, std::size_t count, char_type a) noexcept
    {
        for (std::size_t i = 0; i != count; ++i) {
            p[i] = a;
        }
        return p;
    }

    [[nodiscard]] static constexpr bool eq(char_type a, char_type b) noexcept
    {
        return a == b;
    }

    [[nodiscard]] static constexpr bool lt(char_type a, char_type b) noexcept
    {
        return a < b;
    }

    static constexpr char_type *move(char_type *dst, char_type const *src, std::size_t count) noexcept
    {
        if (src >= dst) {
            for (std::size_t i = 0; i != count; ++i) {
                dst[i] = src[i];
            }
        } else {
            for (std::size_t i = count; i != 0; --i) {
                dst[i - 1] = src[i - 1];
            }
        }
        return dst;
    }

    static constexpr char_type *copy(char_type *dst, char_type const *src, std::size_t count) noexcept
    {
        for (std::size_t i = 0; i != count; ++i) {
            dst[i] = src[i];
        }
        return dst;
    }

    static constexpr int compare(char_type const *s1, char_type const *s2, std::size_t count) noexcept
    {
        for (std::size_t i = 0; i != count; ++i) {
            if (s1[i] != s2[i]) {
                return s1[i] < s2[i] ? -1 : 1;
            }
        }
        return 0;
    }

    static constexpr std::size_t length(char_type const *s) noexcept
    {
        std::size_t i = 0;
        while (s[i].value != 0) {
            ++i;
        }
        return i;
    }

    static constexpr char_type const *find(const char_type *p, std::size_t count, const char_type &ch) noexcept
    {
        for (std::size_t i = 0; i != count; ++i, ++p) {
            if (*p == ch) {
                return p;
            }
        }
        return nullptr;
    }

    static constexpr char_type to_char_type(int_type c) noexcept
    {
        char_type r;
        r.value = c;
        return r;
    }

    static constexpr int_type to_int_type(char_type c) noexcept
    {
        return c.value;
    }

    static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
    {
        return c1 == c2;
    }

    static constexpr int_type eof() noexcept
    {
        return 0;
    }

    static constexpr int_type not_eof(int_type e) noexcept
    {
        return e != 0 ? e : 1;
    }
};

namespace tt::inline v1 {

using gstring = std::basic_string<grapheme>;
using gstring_view = std::basic_string_view<grapheme>;

namespace pmr {
using gstring = std::pmr::basic_string<grapheme>;
}

/** Convert a UTF-32 string to a grapheme-string.
 *
 * @param rhs The UTF-32 string to convert.
 * @param new_line_char The new_line_character to use.
 * @return A grapheme-string.
 */
[[nodiscard]] gstring to_gstring(std::u32string_view rhs, char32_t new_line_char = U'\u2029') noexcept;

/** Convert a UTF-8 string to a grapheme-string.
* 
* @param rhs The UTF-8 string to convert.
* @param new_line_char The new_line_character to use.
* @return A grapheme-string.
 */
[[nodiscard]] inline gstring to_gstring(std::string_view rhs, char32_t new_line_char = U'\u2029') noexcept
{
    return to_gstring(to_u32string(rhs), new_line_char);
}

[[nodiscard]] inline std::string to_string(gstring_view rhs) noexcept
{
    auto r = std::string{};
    r.reserve(rhs.size());
    for (ttlet c: rhs) {
        r += to_string(c);
    }
    return r;
}

[[nodiscard]] inline std::string to_string(gstring const &rhs) noexcept
{
    return to_string(gstring_view{rhs});
}

} // namespace tt::inline v1

template<>
struct std::hash<tt::gstring> {
    [[nodiscard]] std::size_t operator()(tt::gstring const &rhs) noexcept
    {
        auto r = std::hash<std::size_t>{}(rhs.size());
        for (ttlet c: rhs) {
            r = tt::hash_mix_two(r, std::hash<tt::grapheme>{}(c));
        }
        return r;
    }
};

template<>
struct std::hash<tt::pmr::gstring> {
    [[nodiscard]] std::size_t operator()(tt::pmr::gstring const &rhs) noexcept
    {
        auto r = std::hash<std::size_t>{}(rhs.size());
        for (ttlet c : rhs) {
            r = tt::hash_mix_two(r, std::hash<tt::grapheme>{}(c));
        }
        return r;
    }
};
