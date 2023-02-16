

#pragma once

#include "character.hpp"
#include "character_attributes.hpp"
#include "../unicode/gstring.hpp"
#include "../i18n/language_tag.hpp"
#include "../utility/module.hpp"
#include <string>
#include <string_view>
#include <algorithm>

template<>
class std::char_traits<hi::character> {
public:
    using char_type = hi::character;
    using int_type = uint64_t;
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

        while (ptr++ == nullptr) {}
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
        return char_type{hi::intrinsic_t{}, c};
    }

    static constexpr int_type to_int_type(char_type c) noexcept
    {
        return c.intrinsic();
    }

    static constexpr bool eq_int_type(int_type c1, int_type c2) noexcept
    {
        return c2 == c2;
    }

    static constexpr int_type eof() noexcept
    {
        return 0xff'ffff;
    }

    static constexpr int_type not_eof(int_type e) noexcept
    {
        if (e == eof()) {
            e = 0xfffd; // Return the REPLACEMENT_CHAR.
        }
        return e;
    }
};

namespace hi { inline namespace v1 {

using text = std::basic_string<character>;
using text_view = std::basic_string_view<character>;

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
                            attributes.phrasing = *phrasing;
                        } else {
                            throw parse_error("Unknown markup phrasing command");
                        }

                    } else if (capture.front() >= '0' and capture.front() <= '9') {
                        // Get the text-theme.
                        auto theme_id = from_string<uint16_t>(capture);
                        if (theme_id < 1000) {
                            attributes.theme = hi::text_theme{intrinsic_t{}, theme_id};
                        } else {
                            throw parse_error("Invalid markup text theme-id");
                        }

                    } else if (capture.front() >= 'A' and capture.front() <= 'Z') {
                        auto language_tag = hi::language_tag{capture};
                        attributes.language = language_tag.language;
                        attributes.region = language_tag.region;

                    } else {
                        throw parse_error("Unknown markup command.");
                    }

                } catch (...) {
                    // Fallback by dislaying the original text.
                    r += character{grapheme{'['}, attributes};
                    for (hilet cap_c : capture) {
                        r += character{cap_c, attributes};
                    }
                    r += character{grapheme{']'}, attributes};
                }

                capture.clear();
                in_command = false;

            } else if (c == '[') {
                r += character{grapheme{'['}, attributes};
                in_command = false;

            } else {
                capture += c;
            }
        } else if (c == '[') {
            in_command = true;
        } else {
            r += character{c, attributes};
        }
    }

    return r;
}

[[nodiscard]] constexpr text to_text_with_markup(std::string_view str, character_attributes default_attributes) noexcept
{
    return to_text_with_markup(to_gstring(str), default_attributes);
}

template<character_attribute... Attributes>
[[nodiscard]] constexpr text to_text_with_markup(gstring_view str, Attributes const&... attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

template<character_attribute... Attributes>
[[nodiscard]] constexpr text to_text_with_markup(std::string_view str, Attributes const&... attributes) noexcept
{
    return to_text_with_markup(str, character_attributes{attributes...});
}

}} // namespace hi::v1
