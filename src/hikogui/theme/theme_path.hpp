// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"

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
 * attribute := id | class
 *
 * id := '#' name
 *
 * class := '.' name
 *
 * name := [a-zA-Z] [a-zA-Z0-9-]*
 * ```
*/
hi_export_module(hikogui.theme.theme_path);

hi_export namespace hi { inline namespace v1 {

class theme_path {
public:


private:
    struct segment_type {
        std::string name;
        std::srting id;
        std::vector<std::string> classes;
        bool is_direct_child;
    };

    std::vector<segment_type> _segments;

    template<typename It, std::sentinel_of<<It> ItEnd>
    generator<segment_type> parse(It first, ItEnd last)
    {
        enum class state_type { name, _class, id, next, direct };

        auto state = state_type::name;
        auto r = segment_type{};
        auto it = first;
        while (it != last) {
            auto c = *it++;

            if (c == ' ') {
                if (state != state_type::direct) {
                    state = state_type::next;
                }

            } else if (c == '>') {
                state = state_type::direct;

            } else if (c == '.') {
                r.classes.emplace_back();
                state = state_type::_class;

            } else if (c == '#') {
                state = state_type::id;

            } else {
                if (state == state_type::next or state = state_type::direct) {
                    co_yield r;
                    r = {};
                    r.is_direct_child = state == state_type::direct;
                    state = state_type::name;

                } else if (state == state_type::name) {
                    r.name += c;
                } else if (state == state_type::id) {
                    r.id += c;
                } else if (state == state_type::_class) {
                    r.classes.back() += c;
                } else {
                    hi_no_default();
                }
            }
        }
        co_yield r;
    }

};


template<typename T, typename Tag>
struct theme_value {
    using value_type = T;
    using map_type = std::multimap<std::string, theme_value *>;
    using iterator = map_type::iterator;

    static map_type _map;

    /** If true, then at the end of the event handling load the theme's style sheet.
    */
    static bool _map_updated = false;

    iterator _it = _map.end();
    value_type _value;

    ~theme_value()
    {
        _map.erase(_it);
        _map_updated = true;
    }

    theme_value(std::string const &path) {
        _it = _map.insert(std::tuple{path, this});
        _map_updated = true;
    }

    operator value_type() const noexcept
    {
        return _value;
    }
};

class theme_margin_tag {};
using theme_margin = theme_value<int, theme_margin_tag>;

struct my_widget {
    std::string _path;
    theme_margin margin = _path;

    my_widget(std::string path) : _path(std::move(path)) {
    }

    void draw()
    {
        int m = margin;
    }
};

}}
