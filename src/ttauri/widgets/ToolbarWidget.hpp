// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ContainerWidget.hpp"
#include <memory>

namespace tt {

class ToolbarWidget : public ContainerWidget {
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

protected:
    std::vector<Widget *> left_children;
    std::vector<Widget *> right_children;
    relative_base_line child_base_line;
};

} // namespace tt
