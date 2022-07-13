

#include "fancy_text.hpp"

namespace hi::inline v1 {


struct decode_fancy_decoder {
    enum class state_type { text, open, expect_close };

    state_type state = state_type::idle;


    [[nodiscard]] constexpr state_type decode_text(auto it) noexcept
    {
        if (*it == '[') {
            commit_text(it);
            return state_type::open;
        } else {
            return state_type::text;
        }
    }

    [[nodiscard]] constexpr state_type decode_hex_color(auto it) noexcept
    {
        if (*it == ']') {
            text_it = it + 1;
            return state_type::text;
        } else {
            return state_type::hex_color;
        }
    }

    [[nodiscard]] constexpr state_type decode_open(auto it) noexcept
    {
        // clang-format off
        switch (c) {
        case ']': add_char('[');                           return state_type::idle;
        case 'r': phrasing = text_phrasing::regular;       return state_type::expect_close;
        case 'e': phrasing = text_phrasing::emphesis;      return state_type::expect_close;
        case 's': phrasing = text_phrasing::strong;        return state_type::expect_close;
        case 'c': phrasing = text_phrasing::code;          return state_type::expect_close;
        case 'a': phrasing = text_phrasing::abbreviation;  return state_type::expect_close;
        case 'i': phrasing = text_phrasing::italic;        return state_type::expect_close;
        case 'b': phrasing = text_phrasing::bold;          return state_type::expect_close;
        case 'q': phrasing = text_phrasing::quote;         return state_type::expect_close;
        case 'k': phrasing = text_phrasing::keyboard;      return state_type::expect_close;
        case 'h': phrasing = text_phrasing::highlight;     return state_type::expect_close;
        case 'm': phrasing = text_phrasing::math;          return state_type::expect_close;
        case 'u': phrasing = text_phrasing::unarticulated; return state_type::expect_close;
        case '1': phrasing = text_phrasing::link1;         return state_type::expect_close;
        case '2': phrasing = text_phrasing::link2;         return state_type::expect_close;
        case '3': phrasing = text_phrasing::link3;         return state_type::expect_close;
        case '4': phrasing = text_phrasing::link4;         return state_type::expect_close;

        case '#': text_it = it + 1; return state_type::hex_color;

        }
        // clang-format on
    }

    [[nodiscard]] constexpr state_type decode_expect_close(auto it) noexcept
    {
        if (c == ']') {
            [[likely]] return state_type::idle;
        } else {
            // Error condition: ignore it and treat as text.
            text_it = it;
            return state_type::text;
        }
    }

    [[nodiscard]] constexpr state_type decode(auto it) noexcept
    {
        switch (state) {
        case state_type::text:
            return decode_text(it);
        case state_type::open:
            return decode_open(it);
        case state_type::expect_close:
            return decode_expect_close(it);
        }
    }

    constexpr void decode(auto first, auto last) noexcept
    {
        state = state_type::text;
        text_it = first;

        for (auto it = first; it != last; ++it) {
            state = decode(it);
        }
    }

    template<typename It, typename EndIt>
    agstring decode(It first, EndIt last)
    {
        enum class state { idle, text, open };
        state state;

        auto r = agstring{};
        r.reserve(std::distance(first, last));

        for (auto it = first; it != last; ++it) {
            hilet c = *it;

            if (state == state::idle) {
                if (c == '[') {
                    state = state::open;
                } else {
                    text_it = it;
                }

            } else if (state == state::text and c == '[') {
                decode_fancy_text_utf8(text_it, it, cur_language_tag, cur_phrasing, cur_style, r);
                state = state::open;

            } else if (state == text::open) {
                switch (c) {
                case '[':
                    r += agrapheme{'[', cur_language_tag, cur_phrasing, text_style{cur_font_family, cur_font_variant, cur_font_size, cur_color}};
                    state = state::idle;
                    break;

                default:
                    r += agrapheme{0xfffd, cur_language_tag, cur_phrasing, text_style{cur_font_family, cur_font_variant, cur_font_size, cur_color}};
                    state = state::idle;
                    break;
                }
            }

        }

        return r;
    }
};

}

