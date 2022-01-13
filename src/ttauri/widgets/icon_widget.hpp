// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GFX/paged_image.hpp"
#include "../GUI/theme_color.hpp"
#include "../alignment.hpp"
#include "../icon.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt::inline v1 {

/** An simple GUI widget that displays an icon.
 *
 * The icon is scaled to the size of the widget,
 * parent widgets will use this scaling to set the correct size.
 */
class icon_widget final : public widget {
public:
    using super = widget;

    /** The icon to be displayed.
     */
    observable<icon> icon;

    /** The color a non-color icon will be displayed with.
     */
    observable<theme_color> color = theme_color::foreground;

    /** Alignment of the icon inside the widget.
     */
    observable<alignment> alignment = tt::alignment{horizontal_alignment::center, vertical_alignment::middle};

    template<typename Icon, typename Color = tt::theme_color>
    icon_widget(gui_window &window, widget *parent, Icon &&icon, Color &&color = theme_color::foreground) noexcept :
        icon_widget(window, parent)
    {
        this->icon = std::forward<Icon>(icon);
        this->color = std::forward<Color>(color);
    }

    /// @privatesection
    widget_constraints const &set_constraints() noexcept override;
    void set_layout(widget_layout const &layout) noexcept override;
    void draw(draw_context const &context) noexcept override;
    /// @endprivatesection
private:
    enum class icon_type { no, glyph, pixmap };

    icon_type _icon_type;
    glyph_ids _glyph;
    paged_image _pixmap_backing;
    decltype(icon)::callback_ptr_type _icon_callback_ptr;
    std::atomic<bool> _icon_has_modified = true;

    extent2 _icon_size;
    aarectangle _icon_rectangle;

    icon_widget(gui_window &window, widget *parent) noexcept;
};

} // namespace tt::inline v1
