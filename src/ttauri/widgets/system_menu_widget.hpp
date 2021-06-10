// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "icon_widget.hpp"
#include "../GUI/pipeline_image_image.hpp"
#include "../graphic_path.hpp"
#include "../icon.hpp"
#include <memory>
#include <string>
#include <array>


namespace tt {

class system_menu_widget final : public widget {
public:
    using super = widget;

    observable<icon> icon;

    ~system_menu_widget() {}

    system_menu_widget(gui_window &window, std::shared_ptr<widget> parent) noexcept;

    template<typename Icon>
    system_menu_widget(gui_window &window, std::shared_ptr<widget> parent, Icon &&icon) noexcept :
        system_menu_widget(window, std::move(parent))
    {
        this->icon = std::forward<Icon>(icon);

        // Toolbar buttons hug the toolbar and neighbor widgets.
        _margin = 0.0f;
    }

    void init() noexcept override;

    [[nodiscard]] bool
    update_constraints(hires_utc_clock::time_point display_time_point, bool need_reconstrain) noexcept override;
    [[nodiscard]] void update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    [[nodiscard]] hit_box hitbox_test(point2 position) const noexcept override;

private:
    std::shared_ptr<icon_widget> _icon_widget;

    aarectangle system_menu_rectangle;
};

}
