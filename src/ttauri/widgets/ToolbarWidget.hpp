// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget final : public ContainerWidget {
public:
    ToolbarWidget(Window &window, std::shared_ptr<Widget> parent) noexcept;
    ~ToolbarWidget() {}

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual std::shared_ptr<Widget> add_widget(HorizontalAlignment alignment, std::shared_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     */
    template<typename T, HorizontalAlignment Alignment = HorizontalAlignment::Left, typename... Args>
    std::shared_ptr<T> make_widget(Args &&... args)
    {
        auto widget = std::make_shared<T>(window, shared_from_this(), std::forward<Args>(args)...);
        widget->initialize();
        return std::static_pointer_cast<T>(add_widget(Alignment, std::move(widget)));
    }

    [[nodiscard]] bool update_constraints() noexcept override;
    [[nodiscard]] bool update_layout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;

    [[nodiscard]] HitBox hitbox_test(vec window_position) const noexcept override;

private:
    std::vector<std::shared_ptr<Widget>> left_children;
    std::vector<std::shared_ptr<Widget>> right_children;

    flow_layout layout;
    relative_base_line child_base_line;

    void updateConstraintsForChild(Widget const &child, ssize_t index, relative_base_line &shared_base_line, finterval &shared_height) noexcept;

    void updateLayoutForChild(Widget &child, ssize_t index) const noexcept;
};

} // namespace tt
