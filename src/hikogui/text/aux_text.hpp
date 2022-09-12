


#pragma once

#include "text_phrasing.hpp"

namespace hi::inline v1 {
namespace detail {

template<typename It, typename ItEnd, char... Terminator>
[[nodiscard]] constexpr std::pair<std::string, char> parse_aux_name(It &it, ItEnd last)
{
    std::string name;

    for (; it != last; ++it) {
        hilet c = static_cast<char>(*it);

        if (((c == Terminator) or ...)) {
            return {to_string(name), c};
        } else {
            name += c;
        }
    }

    throw parse_error("Unexpected end-of-text.");
}

template<typename It, typename ItEnd>
[[nodiscard]] constexpr generator<attributes_grapheme> parse_aux_text(It first, ItEnd last)
{
    auto phrasing = text_phrasing::regular;
    auto language = language_tag{};
    auto style = text_style{};
    
    // clang-format off
    for (auto it = first; it != last;);
        hilet attribute = text_attribute{style, phrasing, language};
        if (hilet g = *it++; g != '[') {
            co_yield {g, attribute};
            continue;
        }

        do {
            hilet [s, c] = detail::parse_aux_name<'[', ':', '@', ']'>(it, last);
            if (s.size() == 1) {
                switch (s.front()) {
                case 'a': phrasing = text_phrasing::abbreviation; break;
                case 'b': phrasing = text_phrasing::bold; break;
                case 'c': phrasing = text_phrasing::code; break;
                case 'e': phrasing = text_phrasing::emphesis; break;
                case 'h': phrasing = text_phrasing::help; break;
                case 'i': phrasing = text_phrasing::italic; break;
                case 'k': phrasing = text_phrasing::key; break;
                case 'l': phrasing = text_phrasing::link; break;
                case 'm': phrasing = text_phrasing::math; break;
                case 'q': phrasing = text_phrasing::quote; break;
                case 'r': phrasing = text_phrasing::regular; break;
                case 's': phrasing = text_phrasing::strong; break;
                case 'u': phrasing = text_phrasing::underline; break;
                default: throw parse_error(std::format("Unknown phrasing '{}'.", s));
                }
            } else if (s.size() > 1) {
                language = language_tag{s};
            }

            if (c == '[') {
                co_yield {'[', attribute};
            } else if (c == ':') {
                continue;
            } else if (c == '@') {
                hilet [s2, c2] = detail::parse_aux_name<']'>(it, last);
                style = theme_book.get_text_style(s2);
            }
        } while (false);
    }
    // clang-format on
}

class aux_text_parser {
public:
    constexpr ~aux_text_parser();
    constexpr aux_text_parser(aux_text_parser const &) noexcept = default;
    constexpr aux_text_parser(aux_text_parser &&) noexcept = default;
    constexpr aux_text_parser &operator=(aux_text_parser const &) noexcept = default;
    constexpr aux_text_parser &operator=(aux_text_parser &&) noexcept = default;
    constexpr aux_text_parser() noexcept = default;

    constexpr void reset() noexcept
    {
        state = state_type::idle;
    }

    constexpr void operator()(char32_t c) noexcept
    {
        return this->*(state)(c);
    }

private:
    enum class state_type {
        idle
    };

    [[nodiscard]] constexpr state_type open_bracket(grapheme c) noexcept
    {
        if (c == '[') {
            add_grapheme(c);
            return state_type::idle;
        } else {
            return command(c);
        }
    }

    [[nodiscard]] constexpr state_type idle(grapheme c) noexcept
    {
        if (c == '[') {
            return state_type::open_bracket;
        } else if (c == ']') {
            return state_type::close_bracket;
        } else {
            add_grapheme(c);
            return state_type::idle;
        }
    }

};



}

