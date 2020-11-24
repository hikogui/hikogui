// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "widget.hpp"
#include "../GUI/PipelineImage_Image.hpp"
#include "../Path.hpp"
#include "../icon.hpp"
#include "../stencils/image_stencil.hpp"
#include <memory>
#include <string>
#include <array>


namespace tt {

class SystemMenuWidget final : public widget {
public:
    using super = widget;

    SystemMenuWidget(gui_window &window, std::shared_ptr<widget> parent, icon const &icon) noexcept;
    ~SystemMenuWidget() {}

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    void draw(draw_context context, hires_utc_clock::time_point display_time_point) noexcept override;

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override;

private:
    std::unique_ptr<image_stencil> _icon_stencil;

    aarect system_menu_rectangle;
};

}
