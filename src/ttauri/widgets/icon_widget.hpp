// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GFX/draw_context.hpp"
#include "../alignment.hpp"
#include <memory>
#include <string>
#include <array>
#include <optional>
#include <future>

namespace tt {

class icon_widget final : public widget {
public:
    using super = widget;

    observable<icon> icon;
    observable<theme_color> color = theme_color::foreground;
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
    pipeline_image::Image _pixmap_backing;

    aarectangle _icon_bounding_box;
    matrix2 _icon_transform;

    icon_widget(gui_window &window, widget *parent) noexcept;
};

} // namespace tt
