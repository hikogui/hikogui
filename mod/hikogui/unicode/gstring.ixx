// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

module;
#include "../macros.hpp"

#include <vector>
#include <string>
#include <string_view>
#include <type_traits>
#include <iterator>
#include <concepts>
#include <ios>
#include <cuchar>
#include <cwchar>
#include <compare>

export module hikogui_unicode_gstring;
import hikogui_i18n;
import hikogui_time; // XXX #616
import hikogui_unicode_grapheme;
import hikogui_unicode_unicode_grapheme_cluster_break;
import hikogui_unicode_unicode_normalization;
import hikogui_utility;

export namespace std {

template<>
struct char_traits<hi::grapheme> {
    using char_type = hi::grapheme;
    using int_type = std::make_signed_t<char_type::value_type>;
    using off_type = std::streamoff;
    using state_type = std::mbstate_t;
    using pos_type = std::fpos<state_type>;
    using comparison_category = std::strong_ordering;

    constexpr static void assign(char_type& r, char_type const& a) noexcept
    {
        r = a;
    }

    constexpr static char_type *assign(char_type *p, std::size_t count, char_type a) noexcept
    {
        hi_axiom_not_null(p);
        for (std::size_t i = 0; i != count; ++i) {
            p[i] = a;
        }
        return p;
    }

    [[nodiscard]] constexpr static bool eq(char_type a, char_type b) noexcept
    {
        return a == b;
    }

    [[nodiscard]] constexpr static bool lt(char_type a, char_type b) noexcept
    {
        return a < b;
    }

    constexpr static char_type *move(char_type *dst, char_type const *src, std::size_t count) noexcept
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

    constexpr static char_type *copy(char_type *dst, char_type const *src, std::size_t count) noexcept
    {
        hi_axiom_not_null(src);
        hi_axiom_not_null(dst);

        for (std::size_t i = 0; i != count; ++i) {
            dst[i] = src[i];
        }
        return dst;
    }

    constexpr static int compare(char_type const *s1, char_type const *s2, std::size_t count) noexcept
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

    constexpr static std::size_t length(char_type const *s) noexcept
    {
        hi_axiom_not_null(s);

        std::size_t i = 0;
        while (s[i] != '\0') {
            ++i;
        }
        return i;
    }

    constexpr static char_type const *find(const char_type *p, std::size_t count, const char_type& ch) noexcept
    {
        hi_axiom_not_null(p);

        for (std::size_t i = 0; i != count; ++i, ++p) {
            if (*p == ch) {
                return p;
            }
        }
        return nullptr;
    }

    constexpr static char_type to_char_type(int_type c) noexcept
    {
        return c < 0 ? char_type{U'\ufffd'} : char_type{hi::intrinsic_t{}, hi::char_cast<char_type::value_type>(c)};
    }

    constexpr static int_type to_int_type(char_type c) noexcept
    {
        return hi::char_cast<int_type>(c.intrinsic());
    }

    constexpr static bool eq_int_type(int_type c1, int_type c2) noexcept
    {
        return c1 == c2;
    }

    constexpr static int_type eof() noexcept
    {
        return -1;
    }

    constexpr static int_type not_eof(int_type e) noexcept
    {
        return e < 0 ? 0 : e;
    }
};

}

export namespace hi::inline v1 {

using gstring = std::basic_string<grapheme>;
using gstring_view = std::basic_string_view<grapheme>;


[[nodiscard]] constexpr bool operator==(gstring_view const &lhs, std::string_view const &rhs) noexcept
{
    if (lhs.size() != rhs.size()) {
        return false;
    }

    auto l_it = lhs.begin();
    auto l_last = lhs.end();
    auto r_it = rhs.begin();

    for (; l_it != l_last; ++l_it, ++r_it) {
        if (*l_it != *r_it) {
            return false;
        }
    }
    return true;
} 

/** Set the language for the string.
 * 
 * @param first An iterator pointing to the first grapheme.
 * @param last An iterator pointing beyond the last grapheme.
 * @param language The language to set each grapheme to. This language will
 *                 be expanded. The script of the grapheme will never
 *                 contradict the unicode database.
 */
template<std::input_or_output_iterator It, std::sentinel_for<It> ItEnd>
constexpr void set_language(It first, ItEnd last, language_tag language) noexcept
{
    language = language.expand();

    for (auto it = first; it != last; ++it) {
        it->set_language_tag(language);
    }
}

[[nodiscard]] constexpr gstring set_language(gstring str, language_tag language) noexcept
{
    set_language(str.begin(), str.end(), language);
    return str;
}

/** Fix the language for the string.
 * 
 * Make sure that every grapheme will have a language assigned. The following
 * rules will be used to assign languages
 *  - If a grapheme has no language assigned, it will use the language of
 *    the previous grapheme.
 *  - If there is no previous grapheme with an assigned language, the first
 *    language found in the text is used.
 *  - If the text does not have any language assigned then the argument
 *    @a default_language_tag is used.
 *  - If the script of a language contradicts the Unicode Database for the
 *    grapheme then the Unicode Database's script is used instead.
 * 
 * @param first An iterator pointing to the first grapheme.
 * @param last An iterator pointing beyond the last grapheme.
 * @param default_language_tag The language to set each grapheme to.
 */
template<std::input_or_output_iterator It, std::sentinel_for<It> ItEnd>
constexpr void fix_language(It first, ItEnd last, language_tag default_language_tag) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, grapheme>)
{
    if (first == last) {
        return;
    }

    hilet first_language_it = std::find_if(first, last, [](auto &x) { return x.language(); });
    if (first_language_it != last) {
        default_language_tag = first_language_it->language_tag().expand();
    } else {
        default_language_tag = default_language_tag.expand();
    }

    for (auto it = first; it != first_language_it; ++it) {
        it->set_language_tag(default_language_tag);
    }

    for (auto it = first_language_it; it != last; ++it) {
        if (not it->language()) {
            it->set_language_tag(default_language_tag);
        } else {
            it->set_language_tag(it->language_tag().expand());
        }
    }
}

[[nodiscard]] constexpr gstring fix_language(gstring str, language_tag default_language_tag) noexcept
{
    fix_language(str.begin(), str.end(), default_language_tag);
    return str;
}

/** Convert a UTF-32 string-view to a grapheme-string.
 *
 * Before conversion to `gstring` a string is first normalized using the Unicode
 * normalization algorithm. By default it is normalized using NFC.
 *
 * @param rhs The UTF-32 string to convert.
 * @param config The attributes used for normalizing the input string.
 * @return A grapheme-string.
 */
[[nodiscard]] constexpr gstring
to_gstring(std::u32string_view rhs, unicode_normalize_config config = unicode_normalize_config::NFC()) noexcept
{
    hilet normalized_string = unicode_normalize(rhs, config);

    auto r = gstring{};
    auto break_state = detail::grapheme_break_state{};
    auto cluster = std::u32string{};

    for (hilet code_point : normalized_string) {
        if (detail::breaks_grapheme(code_point, break_state)) {
            if (cluster.size() > 0) {
                r += grapheme(composed_t{}, cluster);
            }
            cluster.clear();
        }

        cluster += code_point;
    }
    if (ssize(cluster) != 0) {
        r += grapheme(composed_t{}, cluster);
    }
    return r;
}

/** Convert a UTF-8 string to a grapheme-string.
 *
 * Before conversion to `gstring` a string is first normalized using the Unicode
 * normalization algorithm. By default it is normalized using NFC.
 *
 * @param rhs The UTF-8 string to convert.
 * @param config The attributes used for normalizing the input string.
 * @return A grapheme-string.
 */
[[nodiscard]] constexpr gstring
to_gstring(std::string_view rhs, unicode_normalize_config config = unicode_normalize_config::NFC()) noexcept
{
    return to_gstring(to_u32string(rhs), config);
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

export namespace std {

template<>
struct hash<hi::gstring> {
    [[nodiscard]] std::size_t operator()(hi::gstring const& rhs) noexcept
    {
        auto r = std::hash<std::size_t>{}(rhs.size());
        for (hilet c : rhs) {
            r = hi::hash_mix_two(r, std::hash<hi::grapheme>{}(c));
        }
        return r;
    }
};

}
