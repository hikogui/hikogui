// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "grapheme.hpp"
#include "../utility/module.hpp"
#include "../strings.hpp"
#include <vector>
#include <string>

template<>
struct std::char_traits<hi::grapheme> {
    using char_type = hi::grapheme;
    using int_type = int32_t;
    using off_type = std::streamoff;
    using state_type = std::mbstate_t;
    using pos_type = std::fpos<state_type>;
    using comparison_category = std::strong_ordering;

    static constexpr void assign(char_type& r, char_type const& a) noexcept
    {
        r = a;
    }

    static constexpr char_type *assign(char_type *p, std::size_t count, char_type a) noexcept
    {
        hi_axiom_not_null(p);
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
        hi_axiom_not_null(src);
        hi_axiom_not_null(dst);

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
        hi_axiom_not_null(src);
        hi_axiom_not_null(dst);

        for (std::size_t i = 0; i != count; ++i) {
            dst[i] = src[i];
        }
        return dst;
    }

    static constexpr int compare(char_type const *s1, char_type const *s2, std::size_t count) noexcept
    {
        hi_axiom_not_null(s1);
        hi_axiom_not_null(s2);

        for (std::size_t i = 0; i != count; ++i) {
            if (s1[i] != s2[i]) {
                return s1[i] < s2[i] ? -1 : 1;
            }
        }
        return 0;
    }

    static constexpr std::size_t length(char_type const *s) noexcept
    {
        hi_axiom_not_null(s);

        std::size_t i = 0;
        while (s[i] != '\0') {
            ++i;
        }
        return i;
    }

    static constexpr char_type const *find(const char_type *p, std::size_t count, const char_type& ch) noexcept
    {
        hi_axiom_not_null(p);

        for (std::size_t i = 0; i != count; ++i, ++p) {
            if (*p == ch) {
                return p;
            }
        }
        return nullptr;
    }

    static constexpr char_type to_char_type(int_type c) noexcept
    {
        auto tmp = hi::char_cast<char_type::value_type>(c);
        if (tmp > 0x1f'ffff) {
            tmp = 0;
        }
        return char_type{hi::intrinsic_t{}, tmp};
    }

    static constexpr int_type to_int_type(char_type c) noexcept
    {
        return hi::char_cast<int_type>(c.intrinsic());
    }

    static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
    {
        return c1 == c2;
    }

    static constexpr int_type eof() noexcept
    {
        return -1;
    }

    static constexpr int_type not_eof(int_type e) noexcept
    {
        if (e < 0) {
            e = 0;
        }
        return e;
    }
};

namespace hi::inline v1 {

using gstring = std::basic_string<grapheme>;
using gstring_view = std::basic_string_view<grapheme>;

namespace pmr {
using gstring = std::pmr::basic_string<grapheme>;
}

/** Convert a UTF-32 string-view to a grapheme-string.
 *
 * Before conversion to `gstring` a string is first normalized using the Unicode
 * normalization algorithm. By default it is normalized using NFC.
 *
 * @param rhs The UTF-32 string to convert.
 * @param normalization_mask The attributes used for normalizing the input string.
 * @return A grapheme-string.
 */
[[nodiscard]] gstring
to_gstring(std::u32string_view rhs, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept;

/** Convert a UTF-8 string to a grapheme-string.
 *
 * Before conversion to `gstring` a string is first normalized using the Unicode
 * normalization algorithm. By default it is normalized using NFC.
 *
 * @param rhs The UTF-8 string to convert.
 * @param normalization_mask The attributes used for normalizing the input string.
 * @return A grapheme-string.
 */
[[nodiscard]] inline gstring
to_gstring(std::string_view rhs, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept
{
    return to_gstring(to_u32string(rhs), normalization_mask);
}

/** Convert a UTF-8 string to a grapheme-string.
 *
 * Before conversion to `gstring` a string is first normalized using the Unicode
 * normalization algorithm. By default it is normalized using NFC.
 *
 * @param rhs The UTF-8 string to convert.
 * @param normalization_mask The attributes used for normalizing the input string.
 * @return A grapheme-string.
 */
[[nodiscard]] inline gstring
to_gstring(std::string const& rhs, unicode_normalization_mask normalization_mask = unicode_normalization_mask::NFD) noexcept
{
    return to_gstring(std::string_view{rhs}, normalization_mask);
}

/** Convert a grapheme string to UTF-8.
 *
 * @param rhs The grapheme string view to convert to UTF-8
 * @return The resulting UTF-8 string, in NFC normalization.
 */
[[nodiscard]] constexpr std::string to_string(gstring_view rhs) noexcept
{
    auto r = std::string{};
    r.reserve(rhs.size());
    for (hilet c : rhs) {
        r += to_string(c);
    }
    return r;
}

/** Convert a grapheme string to UTF-8.
 *
 * @param rhs The grapheme string view to convert to UTF-8
 * @return The resulting wide-string, in NFC normalization.
 */
[[nodiscard]] constexpr std::wstring to_wstring(gstring_view rhs) noexcept
{
    auto r = std::wstring{};
    r.reserve(rhs.size());
    for (hilet c : rhs) {
        r += to_wstring(c);
    }
    return r;
}

/** Convert a grapheme string to UTF-8.
 *
 * @param rhs The grapheme string view to convert to UTF-8
 * @return The resulting UTF-32 string, in NFC normalization.
 */
[[nodiscard]] constexpr std::u32string to_u32string(gstring_view rhs) noexcept
{
    auto r = std::u32string{};
    r.reserve(rhs.size());
    for (hilet c : rhs) {
        r += to_u32string(c);
    }
    return r;
}

/** Convert a grapheme string to UTF-8.
 *
 * @param rhs The grapheme string to convert to UTF-8
 * @return The resulting UTF-32 string, in NFC normalization.
 */
[[nodiscard]] constexpr std::string to_string(gstring const& rhs) noexcept
{
    return to_string(gstring_view{rhs});
}

} // namespace hi::inline v1

template<>
struct std::hash<hi::gstring> {
    [[nodiscard]] std::size_t operator()(hi::gstring const& rhs) noexcept
    {
        auto r = std::hash<std::size_t>{}(rhs.size());
        for (hilet c : rhs) {
            r = hi::hash_mix_two(r, std::hash<hi::grapheme>{}(c));
        }
        return r;
    }
};

template<>
struct std::hash<hi::pmr::gstring> {
    [[nodiscard]] std::size_t operator()(hi::pmr::gstring const& rhs) noexcept
    {
        auto r = std::hash<std::size_t>{}(rhs.size());
        for (hilet c : rhs) {
            r = hi::hash_mix_two(r, std::hash<hi::grapheme>{}(c));
        }
        return r;
    }
};
