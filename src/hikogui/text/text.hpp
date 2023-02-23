

#pragma once

#include "character.hpp"
#include "character_attributes.hpp"
#include "../unicode/gstring.hpp"
#include "../unicode/unicode_description.hpp"
#include "../i18n/language_tag.hpp"
#include "../utility/module.hpp"
#include "../os_settings.hpp"
#include <string>
#include <string_view>
#include <algorithm>
#include <iterator>
#include <ranges>

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

        while (ptr++ != '\0') {}
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
 */
template<typename It, std::sentinel_for<It> ItEnd>
inline void fixup_script(It first, ItEnd last) noexcept
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
            hilet& udb = unicode_description::find(code_point);
            hilet udb_script = udb.script();

            if (udb_script != unicode_script::Zzzz and udb_script != unicode_script::Common) {
                // This character is defined in the Unicode database to have a
                // specific script.
                return iso_15924{udb_script};

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
    last_script = os_settings::default_script();
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

template<std::ranges::range R>
inline void fixup_script(R& str) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return fixup_script(std::ranges::begin(str), std::ranges::end(str));
}

[[nodiscard]] constexpr text to_text_with_markup(gstring_view str, character_attributes default_attributes) noexcept
{
    auto r = text{};
    auto attributes = default_attributes;

    auto in_command = false;
    auto capture = std::string{};
    for (hilet c : str) {
        if (in_command) {
            if (c == ']') {
                try {
                    if (capture.empty()) {
                        throw parse_error("Empty markup command.");

                    } else if (capture.size() == 1) {
                        if (capture.front() == '.') {
                            attributes = default_attributes;
                        } else if (auto phrasing = to_text_phrasing(capture.front())) {
                            attributes.set_phrasing(*phrasing);
                        } else {
                            throw parse_error("Unknown markup phrasing command");
                        }

                    } else if (
                        (capture.front() >= 'a' and capture.front() <= 'z') or
                        (capture.front() >= 'A' and capture.front() <= 'Z')) {
                        attributes.set_language(hi::language_tag{capture}.expand());

                    } else {
                        throw parse_error("Unknown markup command.");
                    }

                } catch (...) {
                    // Fallback by displaying the original text.
                    r += character{'[', attributes};
                    for (hilet cap_c : capture) {
                        r += character{cap_c, attributes};
                    }
                    r += character{']', attributes};
                }

                capture.clear();
                in_command = false;

            } else if (c == '[') {
                r += character{'[', attributes};
                in_command = false;

            } else if (c.size() == 1 and c[0] <= 0x7f) {
                capture += char_cast<char>(c[0]);

            } else {
                throw parse_error("Unexpected non-ASCII character in markup command.");
            }
        } else if (c == '[') {
            in_command = true;
        } else {
            r += character{c, attributes};
        }
    }

    fixup_script(r);
    return r;
}

[[nodiscard]] constexpr text to_text_with_markup(std::string_view str, character_attributes default_attributes) noexcept
{
    return to_text_with_markup(to_gstring(str), default_attributes);
}

template<character_attribute... Attributes>
[[nodiscard]] constexpr text to_text_with_markup(gstring_view str, Attributes const&...attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

template<character_attribute... Attributes>
[[nodiscard]] constexpr text to_text_with_markup(std::string_view str, Attributes const&...attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

/** Change the attributes on a piece of text.
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
 */
template<typename It, std::sentinel_for<It> ItEnd, character_attribute... Args>
inline void set_attributes(It first, ItEnd last, Args const&...args) noexcept
    requires(std::is_same_v<std::iter_value_t<It>, character>)
{
    return set_attributes(first, last, character_attributes{args...})
}

/** Change the attributes on text.
 */
template<std::ranges::range R>
inline void set_attributes(R& str, character_attributes attributes) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return set_attributes(std::ranges::begin(str), std::ranges::end(str), attributes);
}

/** Change the attributes on text.
 */
template<std::ranges::range R, character_attribute... Args>
inline void set_attributes(R& str, Args const&...args) noexcept
    requires(std::is_same_v<std::ranges::range_value_t<R>, character>)
{
    return set_attributes(str, character_attributes{args...});
}

}} // namespace hi::v1
