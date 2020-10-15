// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget final : public ContainerWidget {
public:
    ToolbarWidget(Window &window, Widget *parent) noexcept;
    ~ToolbarWidget() {}

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual Widget &addWidget(HorizontalAlignment alignment, std::unique_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     */
    template<typename T, HorizontalAlignment Alignment = HorizontalAlignment::Left, typename... Args>
    T &makeWidget(Args &&... args)
    {
        return static_cast<T &>(addWidget(Alignment, std::make_unique<T>(window, this, std::forward<Args>(args)...)));
    }

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept;
    void draw(DrawContext context, hires_utc_clock::time_point display_time_point) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override;

private:
    std::vector<Widget *> left_children;
    std::vector<Widget *> right_children;

    flow_layout layout;
    relative_base_line child_base_line;

    void updateConstraintsForChild(Widget const &child, ssize_t index, relative_base_line &shared_base_line, finterval &shared_height) noexcept;

    void updateLayoutForChild(Widget &child, ssize_t index) const noexcept;
};

} // namespace tt
