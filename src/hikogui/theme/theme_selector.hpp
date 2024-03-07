// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <iterator>

/** @module hikogui.theme.theme_path
 *
 * A theme-path is used to select theme-values for a specific widget.
 *
 * The path describes the construction of widgets and their children.
 *
 * For example `button#this.right label@margin-left` path describes the label-widget that is part
 * of the button-widget.
 *
 * Path BNF:
 *
 * ```
 * path := widget ( ' ' widget )* '@'
 *
 * widget := name attribute*
 *
 * attribute := id | class | pseudo
 *
 * id := '#' name
 *
 * class := '.' name
 *
 * pseudo := ':' name
 *
 * name := [a-zA-Z] [a-zA-Z0-9-]*
 * ```
 */
hi_export_module(hikogui.theme.theme_path);

hi_export namespace hi {
inline namespace v1 {

struct theme_selector_element {
    std::string name = {};
    std::string id = {};
    std::vector<std::string> classes = {};
    std::vector<std::string> pseudos = {};
    bool is_direct_parent = false;

    constexpr void add_class(std::string rhs)
    {
        auto it = std::lower_bound(classes.cbegin(), classes.cend(), rhs);
        if (it != classes.cend() and *it == rhs) {
            return;
        }
        classes.insert(it, std::move(rhs));
    }

    constexpr void add_pseudo(std::string rhs)
    {
        auto it = std::lower_bound(pseudos.cbegin(), pseudos.cend(), rhs);
        if (it != pseudos.cend() and *it == rhs) {
            return;
        }
        pseudos.insert(it, std::move(rhs));
    }

    [[nodiscard]] constexpr friend bool operator==(theme_selector_element const& lhs, theme_selector_element const& rhs) noexcept
    {
        if (lhs.name != rhs.name) {
            return false;
        }
        if (lhs.id != rhs.id) {
            return false;
        }

        if (lhs.classes.size() != rhs.classes.size()) {
            return false;
        }

        return lhs.is_direct_parent == rhs.is_direct_parent;
    }

    [[nodiscard]] constexpr friend bool
    match(theme_selector_element const& needle, theme_selector_element const& haystack) noexcept
    {
        if (not needle.name.empty() and needle.name != haystack.name) {
            return false;
        }
        if (not needle.id.empty() and needle.id != haystack.id) {
            return false;
        }
        for (auto const& class_ : needle.classes) {
            if (std::find(haystack.classes.begin(), haystack.classes.end(), class_) == haystack.classes.end()) {
                return false;
            }
        }
        for (auto const& pseudo : needle.pseudos) {
            if (std::find(haystack.pseudos.begin(), haystack.pseudos.end(), pseudo) == haystack.pseudos.end()) {
                return false;
            }
        }
        return true;
    }
};

class theme_selector : public std::vector<theme_selector_element> {};

[[nodiscard]] constexpr bool match(
    theme_selector::const_iterator needle_first,
    theme_selector::const_iterator needle_last,
    theme_selector::const_iterator haystack_first,
    theme_selector::const_iterator haystack_last,
    bool is_direct_child) noexcept
{
    if (needle_first == needle_last) {
        // The match is successful if the last element of the needle matches the
        // last element of the haystack.
        return haystack_first == haystack_last;
    }

    while (haystack_first != haystack_last) {
        if (match(*needle_first, *haystack_first)) {
            // The first element matches, continue with the next element.
            return match(needle_first + 1, needle_last, haystack_first + 1, haystack_last, needle_first->is_direct_parent);
        }

        if (is_direct_child) {
            // This element should have directly matched (parent/child).
            return false;
        }

        // This element may still match later on (a descendant).
        ++haystack_first;
    }

    hi_axiom(needle_first != needle_last);
    return false;
}

[[nodiscard]] constexpr bool match(theme_selector const& needle, theme_selector const& haystack) noexcept
{
    return match(needle.begin(), needle.end(), haystack.begin(), haystack.end(), false);
}

template<std::input_iterator ItIn, std::sentinel_for<ItIn> ItEnd, std::output_iterator<theme_selector_element> ItOut>
constexpr void parse_theme_selector(ItIn first, ItEnd last, ItOut out) noexcept
{
    enum class state_type { name, _class, id, pseudo, space };

    auto state = state_type::name;
    auto r = theme_selector_element{};
    auto element_used = false;
    auto buffer = std::string{};

    auto flush_buffer = [&] () {
        if (buffer.empty()) {
            return;
        }
        if (state == state_type::name) {
            r.name = std::exchange(buffer, {});
        } else if (state == state_type::id) {
            r.id = std::exchange(buffer, {});
        } else if (state == state_type::_class) {
            r.add_class(std::exchange(buffer, {}));
        } else if (state == state_type::pseudo) {
            r.add_pseudo(std::exchange(buffer, {}));
        } else {
            hi_no_default();
        }
    };

    for (auto it = first; it != last; ++it) {
        if (*it == ' ' or *it == '\t') {
            flush_buffer();
            state = state_type::space;

        } else if (*it == '>') {
            flush_buffer();
            state = state_type::space;
            r.is_direct_parent = true;

        } else {
            if (state == state_type::space) {
                if (element_used) {
                    *out++ = std::exchange(r, {});
                }
                state = state_type::name;
            }

            element_used = true;
            if (*it == '.') {
                flush_buffer();
                state = state_type::_class;

            } else if (*it == ':') {
                flush_buffer();
                state = state_type::pseudo;

            } else if (*it == '#') {
                flush_buffer();
                state = state_type::id;

            } else if (*it == '*') {
                state = state_type::name;

            } else {
                buffer += *it;
            }
        }
    }

    flush_buffer();
    if (element_used) {
        *out++ = r;
    }
}

[[nodiscard]] constexpr theme_selector parse_theme_selector(std::string_view str) noexcept
{
    auto r = theme_selector{};
    parse_theme_selector(str.begin(), str.end(), std::back_inserter(r));
    return r;
}

} // namespace v1
}

template<>
struct std::formatter<hi::theme_selector_element, char> : std::formatter<std::string, char> {
    auto format(hi::theme_selector_element const& t, auto& fc) const
    {
        auto r = std::string{};

        if (not t.name.empty()) {
            r = t.name;
        }

        if (not t.id.empty()) {
            r += '#';
            r += t.id;
        }

        for (auto const& class_ : t.classes) {
            r += '.';
            r += class_;
        }

        for (auto const& pseudo : t.pseudos) {
            r += ':';
            r += pseudo;
        }

        if (r.empty()) {
            r = "*";
        }

        if (t.is_direct_parent) {
            r += " >";
        }

        return std::formatter<std::string, char>::format(std::move(r), fc);
    }
};

template<>
struct std::formatter<hi::theme_selector, char> : std::formatter<std::string, char> {
    auto format(hi::theme_selector const& t, auto& fc) const
    {
        auto r = std::string{};

        for (auto const& element : t) {
            if (not r.empty()) {
                r += ' ';
            }
            r += std::format("{}", element);
        }

        return std::formatter<std::string, char>::format(std::move(r), fc);
    }
};
