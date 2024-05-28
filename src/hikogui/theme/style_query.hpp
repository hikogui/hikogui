// Copyright Take Vos 2024.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "style_attributes.hpp"
#include "style_pseudo_class.hpp"
#include "style_path.hpp"
#include <typeinfo>

hi_export_module(hikogui.theme : query_style_attributes);

hi_export namespace hi {
inline namespace v1 {

/** Query for style attributes.
 * 
 * The theme system will create a style_query subclass to make a
 * style_attributes object for a specific style_path and pseudo_class.
 */
struct style_query {
    virtual ~style_query() noexcept = default;

    /** Compare style-queries
     * 
     * For performance reasons the theme system should supply a comparison
     * to check if the theme has changed. Possibly the theme system may reload
     * a theme from disk and the values may have changed. You could in that
     * case use the file-modification-date to compare the themes, to make
     * this fast.
     */
    [[nodiscard]] virtual bool operator==(style_query const&rhs) const noexcept = 0;

    /** Query style attributes of a theme for a specific path and pseudo-class.
     * 
     * @param path The widget-path to query the style for.
     * @param pseudo_class The pseudo-class (state of the widget).
     * @return The style attributes for the path and pseudo-class.
     */
    [[nodiscard]] virtual style_attributes get_attributes(style_path const&path, style_pseudo_class pseudo_class) const = 0;
};

} // namespace v1
} // namespace hi

