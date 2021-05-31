// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "label_delegate.hpp"
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

    template<typename... Args>
    icon_widget(
        gui_window &window,
        std::shared_ptr<widget> parent,
        std::shared_ptr<label_delegate> delegate,
        alignment alignment,
        tt::icon const &icon) noexcept :
        super(window, std::move(parent), std::move(delegate)), _alignment(alignment)
    {
        set_icon(icon);
    }

    template<typename... Args>
    icon_widget(gui_window &window, std::shared_ptr<widget> parent, tt::icon icon) noexcept :
        icon_widget(window, std::move(parent), std::make_shared<label_delegate>(), alignment::middle_center, std::move(icon))
    {
    }

    ~icon_widget();

    tt::icon icon() const noexcept;

    void set_icon(tt::icon const &icon) noexcept;

    [[nodiscard]] bool update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;

    [[nodiscard]] void update_layout(hires_utc_clock::time_point displayTimePoint, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

private:
    enum class icon_type { no, glyph, pixmap };

    alignment _alignment;

    icon_type _icon_type;
    font_glyph_ids _glyph;
    size_t _pixmap_hash;
    pipeline_image::Image _pixmap_backing;

    aarectangle _icon_bounding_box;
    matrix2 _icon_transform;
};

} // namespace tt
