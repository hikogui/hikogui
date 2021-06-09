// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "../GUI/draw_context.hpp"
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

    icon_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        alignment alignment = alignment::middle_center) noexcept :
        super(window, std::move(parent)), _alignment(alignment)
    {
    }

    template<typename Icon>
    icon_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        alignment alignment,
        Icon &&icon) noexcept :
        icon_widget(window, std::move(parent), alignment)
    {
        icon = std::forward<Icon>(icon);
    }

    ~icon_widget();

    void init() noexcept override;

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

private:
    enum class icon_type { no, glyph, pixmap };

    decltype(icon)::callback_ptr_type _icon_callback;

    alignment _alignment;

    icon_type _icon_type;
    font_glyph_ids _glyph;
    size_t _pixmap_hash;
    pipeline_image::Image _pixmap_backing;

    aarectangle _icon_bounding_box;
    matrix2 _icon_transform;
};

} // namespace tt
