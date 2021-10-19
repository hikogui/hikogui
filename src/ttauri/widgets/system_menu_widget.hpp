// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "widget.hpp"
#include "icon_widget.hpp"
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

    system_menu_widget(gui_window &window, widget *parent) noexcept;

    template<typename Icon>
    system_menu_widget(gui_window &window, widget *parent, Icon &&icon) noexcept : system_menu_widget(window, parent)
    {
        this->icon = std::forward<Icon>(icon);
    }

    /// @privatesection
    [[nodiscard]] pmr::generator<widget *> children(std::pmr::polymorphic_allocator<> &) const noexcept override
    {
        co_yield _icon_widget.get();
    }

    [[nodiscard]] float margin() const noexcept override;
    [[nodiscard]] bool constrain(utc_nanoseconds display_time_point, bool need_reconstrain) noexcept override;
    void layout(matrix3 const &to_window, extent2 const &new_size, utc_nanoseconds display_time_point, bool need_layout) noexcept override;
    [[nodiscard]] hitbox hitbox_test(point2 position) const noexcept override;
    /// @endprivatesection
private:
    std::unique_ptr<icon_widget> _icon_widget;

    aarectangle system_menu_rectangle;
};

} // namespace tt
