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
 * For example `/button/label` path describes the label-widget that is part
 * of the button-widget. 
*/
hi_export_module(hikogui.theme.theme_path);

hi_export namespace hi { inline namespace v1 {

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
