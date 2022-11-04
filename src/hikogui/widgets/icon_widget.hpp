// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file widgets/icon_widget.hpp Defines icon_widget.
 * @ingroup widgets
 */

#pragma once

#include "widget.hpp"
#include "../GFX/paged_image.hpp"
#include "../alignment.hpp"
#include "../label.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace hi { inline namespace v1 {

template<typename Context>
concept icon_widget_attribute = forward_of<Context, observer<hi::icon>, observer<hi::alignment>, observer<hi::color>>;

/** An simple GUI widget that displays an icon.
 * @ingroup widgets
 *
 * The icon is scaled to the size of the widget,
 * parent widgets will use this scaling to set the correct size.
 */
class icon_widget final : public widget {
public:
    using super = widget;

    /** The icon to be displayed.
     */
    observer<icon> icon = hi::icon{};

    /** The color a non-color icon will be displayed with.
     */
    observer<color> color = color::foreground();

    /** Alignment of the icon inside the widget.
     */
    observer<alignment> alignment = hi::alignment::middle_center();

    icon_widget(widget *parent, icon_widget_attribute auto&&...attributes) noexcept :
        icon_widget(parent)
    {
        set_attributes(hi_forward(attributes)...);
    }

    void set_attributes() noexcept {}
    void set_attributes(icon_widget_attribute auto&& first, icon_widget_attribute auto&&...rest) noexcept
    {
        if constexpr (forward_of<decltype(first), observer<hi::icon>>) {
            icon = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::alignment>>) {
            alignment = hi_forward(first);
        } else if constexpr (forward_of<decltype(first), observer<hi::color>>) {
            color = hi_forward(first);
        } else {
            hi_static_no_default();
        }
        set_attributes(hi_forward(rest)...);
    }

    /// @privatesection
    widget_constraints const& set_constraints(set_constraints_context const& context) noexcept override;
    void set_layout(widget_layout const& context) noexcept override;
    void draw(draw_context const& context) noexcept override;
    /// @endprivatesection
private:
    enum class icon_type { no, glyph, pixmap };

    icon_type _icon_type;
    glyph_ids _glyph;
    paged_image _pixmap_backing;
    decltype(icon)::callback_token _icon_cbt;
    std::atomic<bool> _icon_has_modified = true;

    extent2 _icon_size;
    aarectangle _icon_rectangle;

    icon_widget(widget *parent) noexcept;
};

}} // namespace hi::v1
