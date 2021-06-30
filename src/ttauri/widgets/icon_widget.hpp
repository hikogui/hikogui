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
    observable<alignment> alignment = alignment::middle_center;
    observable<theme_color> color = theme_color::foreground;

    icon_widget(
        gui_window &window,
        widget *parent) noexcept :
        super(window, parent)
    {
    }

    template<typename Icon>
    icon_widget(
        gui_window &window,
        widget *parent,
        Icon &&icon) noexcept :
        icon_widget(window, parent)
    {
        this->icon = std::forward<Icon>(icon);
    }

    ~icon_widget();

    void init() noexcept override;

    [[nodiscard]] bool constrain(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

private:
    enum class icon_type { no, glyph, pixmap };

    decltype(icon)::callback_ptr_type _icon_callback;

    icon_type _icon_type;
    font_glyph_ids _glyph;
    size_t _pixmap_hash;
    pipeline_image::Image _pixmap_backing;

    aarectangle _icon_bounding_box;
    matrix2 _icon_transform;
};

} // namespace tt
