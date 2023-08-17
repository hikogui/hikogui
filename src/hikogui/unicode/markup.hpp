

#pragma once

#include "character.hpp"
#include "character_attributes.hpp"
#include "../unicode/module.hpp"
#include "../i18n/module.hpp"
#include "../utility/module.hpp"
#include <string>
#include <string_view>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <concepts>

/** Character-traits for `hi::character`.
 *
 * These character traits allow `std::basic_string` and `std::basic_string_view`
 * to work with characters to form `hi::text` and `hi::text_view`.
 *
 * @ingroup text
 */
template<>
class std::char_traits<hi::character> {
public:
    using char_type = hi::character;
    using int_type = int64_t;
    using off_type = size_t;
    using pos_type = size_t;

    static constexpr void assign(char_type& r, char_type const& a) noexcept
    {
        r = a;
    }

    static constexpr char_type *assign(char_type *p, size_t count, char_type a) noexcept
    {
        std::fill_n(p, count, a);
        return p;
    }

    static constexpr bool eq(char_type a, char_type b) noexcept
    {
        return a == b;
    }

    static constexpr bool lt(char_type a, char_type b) noexcept
    {
        return a < b;
    }

    static constexpr char_type *move(char_type *dest, const char_type *src, std::size_t count) noexcept
    {
        auto first = src;
        auto last = src + count;

        if (dest >= first and dest < last) {
            std::copy_backward(first, last, dest);
        } else {
            std::copy(first, last, dest);
        }
        return dest;
    }

    static constexpr char_type *copy(char_type *dest, const char_type *src, std::size_t count) noexcept
    {
        std::copy_n(src, count, dest);
        return dest;
    }

    static constexpr int compare(const char_type *s1, const char_type *s2, std::size_t count) noexcept
    {
        auto r = std::lexicographical_compare_three_way(s1, s1 + count, s2, s2 + count);
        return r == std::strong_ordering::equal ? 0 : r == std::strong_ordering::less ? -1 : 1;
    }

    static constexpr std::size_t length(const char_type *s) noexcept
    {
        auto ptr = s;

        while (*ptr++ != '\0') {}
        return std::distance(s, ptr);
    }

    static constexpr const char_type *find(const char_type *p, std::size_t count, const char_type& ch) noexcept
    {
        auto first = p;
        auto last = p + count;

        auto it = std::find(first, last, ch);
        if (it == last) {
            it = nullptr;
        }
        return it;
    }

    static constexpr char_type to_char_type(int_type c) noexcept
    {
        auto tmp = hi::char_cast<char_type::value_type>(c);
        if (c < 0) {
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
        return c2 == c2;
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

namespace hi { inline namespace v1 {

using text = std::basic_string<character>;
using text_view = std::basic_string_view<character>;

/** Fixup the iso_15924 script in text.
 *
 * Check the characters in text and make sure the script-attribute does not
 * contradict the Unicode script table. And if the script-attribute for the
 * character was not set, then determine the script.
 *
 * @ingroup text
 */
template<typename It, std::sentinel_for<It> ItEnd>
inline void fixup_script(It first, ItEnd last, iso_15924 default_script) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, character>)
{
    // Overwrite the script of a character if it is specific in the Unicode
    // script table.
    auto last_language = iso_639{};
    auto last_script = iso_15924{};
    auto missing_script_count = 0_uz;
    for (auto it = first; it != last; ++it) {
        hilet code_point = (*it)[0];
        auto attributes = it->attributes();
        hilet language = attributes.language();
        hilet script = attributes.script();

        // Reset the last script when the language changes.
        if (language != last_language) {
            last_language = language;
            last_script = {};
        }

        hilet new_script = [&] {
            hilet udb_script = ucd_get_script(code_point);

            if (udb_script != unicode_script::Zzzz and udb_script != unicode_script::Common) {
                // This character is defined in the Unicode database to have a
                // specific script.
                return udb_script;

            } else if (not script and last_script) {
                // This character did not have a script, but a previous character
                // from the same language did have a script.
                return last_script;

            } else {
                return script;
            }
        }();

        // If the new script is different update the character.
        if (script != new_script) {
            attributes.set_script(new_script);
            it->set_attributes(attributes);
        }

        // We found a script for the character, remember it.
        if (new_script) {
            last_script = new_script;
        } else {
            ++missing_script_count;
        }
    }

    if (missing_script_count == 0) {
        // Every character has a script assigned.
        return;
    }

    // In the second iteration we search backwards for scripts and assign them.
    // Since in this iteration we have to assign a script we don't care about
    // the language. And we fallback to the operating system's default script.
    last_script = default_script;
    for (auto rev_it = last; rev_it != first; --rev_it) {
        hilet it = rev_it - 1;
        auto attributes = it->attributes();
        hilet script = attributes.script();

        hilet new_script = script ? script : last_script;

        // If the new script is different update the character.
        if (script != new_script) {
            attributes.set_script(script);
            it->set_attributes(attributes);
        }

        // We found a script for the character, remember it.
        if (new_script) {
            last_script = new_script;
        }
    }
}

/**
 * @ingroup text
 */
template<std::ranges::range R>
inline void fixup_script(R& str, iso_15924 default_script) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return fixup_script(std::ranges::begin(str), std::ranges::end(str), default_script);
}

/**
 * @ingroup text
 */
[[nodiscard]] inline text to_text(gstring_view str, character_attributes default_attributes) noexcept
{
    auto r = text{};
    r.reserve(str.size());
    for (hilet c : str) {
        r += character{c, default_attributes};
    }
    return r;
}

/**
 * @ingroup text
 */
template<character_attribute... Args>
[[nodiscard]] inline text to_text(gstring_view str, Args const&...args) noexcept
{
    return to_text(str, character_attributes{args...});
}

/**
 * @ingroup text
 */
[[nodiscard]] inline text to_text(
    std::string_view str,
    unicode_normalize_config config,
    character_attributes default_attributes) noexcept
{
    return to_text(to_gstring(str, config), default_attributes);
}

/**
 * @ingroup text
 */
[[nodiscard]] inline text
to_text(std::string_view str, character_attributes default_attributes) noexcept
{
    return to_text(to_gstring(str, unicode_normalize_config::NFC()), default_attributes);
}


/**
 * @ingroup text
 */
template<character_attribute... Args>
[[nodiscard]] inline text to_text(std::string_view str, unicode_normalize_config config, Args const&...args) noexcept
{
    return to_text(str, config, character_attributes{args...});
}

/**
 * @ingroup text
 */
template<character_attribute... Args>
[[nodiscard]] inline text to_text(std::string_view str, Args const&...args) noexcept
{
    return to_text(str, unicode_normalize_config::NFC(), character_attributes{args...});
}

/**
 * @ingroup text
 */
[[nodiscard]] inline text to_text_with_markup(gstring_view str, character_attributes default_attributes) noexcept
{
    auto r = text{};
    auto attributes = default_attributes;

    auto in_command = false;
    auto capture = std::string{};

    auto output_incomplete_command = [&] {
        r += character{'[', attributes};
        for (hilet cap_c : capture) {
            r += character{cap_c, attributes};
        }
    };

    for (hilet c : str) {
        if (in_command) {
            if (c == ']') {
                in_command = false;

                auto output_bad_command = [&] {
                    output_incomplete_command();
                    r += character{']', attributes};
                };

                if (capture.empty()) {
                    output_bad_command();

                } else if (capture.size() == 1) {
                    if (capture.front() == '.') {
                        attributes = default_attributes;
                    } else if (auto phrasing = to_text_phrasing(capture.front())) {
                        attributes.set_phrasing(*phrasing);
                    } else {
                        output_bad_command();
                    }

                } else if (is_alpha(capture.front())) {
                    try {
                        attributes.set_language_tag(hi::language_tag{capture}.expand());
                    } catch (...) {
                        output_bad_command();
                    }

                } else {
                    output_bad_command();
                }
                capture.clear();

            } else if (c == '[' and capture.empty()) {
                in_command = false;
                r += character{'[', attributes};

            } else if (c.size() == 1 and c[0] <= 0x7f) {
                capture += char_cast<char>(c[0]);

            } else {
                in_command = false;
                output_incomplete_command();
                capture.clear();
            }

        } else if (c == '[') {
            in_command = true;

        } else {
            r += character{c, attributes};
        }
    }

    if (not capture.empty()) {
        output_incomplete_command();
    }

    fixup_script(r, default_attributes.script());
    return r;
}

/**
 * @ingroup text
 */
[[nodiscard]] inline text to_text_with_markup(std::string_view str, character_attributes default_attributes) noexcept
{
    return to_text_with_markup(to_gstring(str, unicode_normalize_config::NFC_PS_noctr()), default_attributes);
}

/**
 * @ingroup text
 */
template<character_attribute... Attributes>
[[nodiscard]] inline text to_text_with_markup(gstring_view str, Attributes const&...attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

/**
 * @ingroup text
 */
template<character_attribute... Attributes>
[[nodiscard]] inline text to_text_with_markup(std::string_view str, Attributes const&...attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

/**
 * @ingroup text
 */
[[nodiscard]] constexpr gstring to_gstring(text_view str) noexcept
{
    auto r = gstring{};
    r.reserve(str.size());
    for (hilet c : str) {
        r += c.grapheme();
    }
    return r;
}

/**
 * @ingroup text
 */
[[nodiscard]] constexpr std::string to_string(text_view str) noexcept
{
    return to_string(to_gstring(str));
}

/**
 * @ingroup text
 */
[[nodiscard]] constexpr std::wstring to_wstring(text_view str) noexcept
{
    return to_wstring(to_gstring(str));
}

/**
 * @ingroup text
 */
[[nodiscard]] constexpr std::u32string to_u32string(text_view str) noexcept
{
    return to_u32string(to_gstring(str));
}

/** Change the attributes on a piece of text.
 * @ingroup text
 */
template<typename It, std::sentinel_for<It> ItEnd>
inline void set_attributes(It first, ItEnd last, character_attributes attributes) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, character>)
{
    for (auto it = first; it != last; ++it) {
        it->set_attributes(attributes);
    }
    fixup_script(first, last);
}

/** Change the attributes on a piece of text.
 * @ingroup text
 */
template<typename It, std::sentinel_for<It> ItEnd, character_attribute... Args>
inline void set_attributes(It first, ItEnd last, Args const&...args) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, character>)
{
    return set_attributes(first, last, character_attributes{args...});
}

/** Change the attributes on text.
 * @ingroup text
 */
template<std::ranges::range R>
inline void set_attributes(R& str, character_attributes attributes) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return set_attributes(std::ranges::begin(str), std::ranges::end(str), attributes);
}

/** Change the attributes on text.
 * @ingroup text
 */
template<std::ranges::range R, character_attribute... Args>
inline void set_attributes(R& str, Args const&...args) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return set_attributes(str, character_attributes{args...});
}

/** Convert integer to string.
 * This function bypasses std::locale.
 *
 * @ingroup text
 * @param value The signed or unsigned integer value.
 * @return The integer converted to a decimal string.
 */
[[nodiscard]] text to_text(std::integral auto const& value) noexcept
{
    return to_text(to_string(value));
}

/** Convert floating point to string.
 * This function bypasses std::locale.
 *
 * @ingroup text
 * @param value The signed or unsigned integer value.
 * @return The integer converted to a decimal string.
 */
[[nodiscard]] text to_text(std::floating_point auto const& value) noexcept
{
    return to_text(to_string(value));
}

/** Convert a string to an integer.
 * This function bypasses std::locale
 *
 * @ingroup text
 * @tparam T The integer type.
 * @param str The string is an integer.
 * @param base The base radix of the string encoded integer.
 * @return The integer converted from a string.
 */
template<std::integral T>
[[nodiscard]] T from_text(text_view str, int base = 10)
{
    return from_string<T>(to_string(str), base);
}

/** Convert a string to an floating point.
 * This function bypasses std::locale
 *
 * @ingroup text
 * @tparam T The integer type.
 * @param str The string is an integer.
 * @return The integer converted from a string.
 */
template<std::floating_point T>
[[nodiscard]] T from_text(text_view str)
{
    return from_string<T>(to_string(str));
}

}} // namespace hi::v1
