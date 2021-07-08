// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GFX/draw_context.hpp"
#include "../GFX/pipeline_image_image.hpp"
#include "../GUI/theme_color.hpp"
#include "../alignment.hpp"
#include "../icon.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

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
    observable<alignment> alignment = alignment::middle_center;

    template<typename Icon, typename Color = tt::theme_color>
    icon_widget(
        gui_window &window,
        widget *parent, Icon &&icon, Color &&color = theme_color::foreground) noexcept :
        icon_widget(window, parent)
    {
        this->icon = std::forward<Icon>(icon);
        this->color = std::forward<Color>(color);
    }

    /// @privatesection
    void init() noexcept override;
    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;
    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;
    /// @endprivatesection
private:
    enum class icon_type { no, glyph, pixmap };

    decltype(icon)::callback_ptr_type _icon_callback;

    icon_type _icon_type;
    font_glyph_ids _glyph;
    size_t _pixmap_hash;
    pipeline_image::image _pixmap_backing;

    aarectangle _icon_bounding_box;
    matrix2 _icon_transform;

    icon_widget(gui_window &window, widget *parent) noexcept;
};

} // namespace tt
