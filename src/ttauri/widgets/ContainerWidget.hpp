// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace tt {

class ContainerWidget : public Widget {
protected:
    std::vector<std::unique_ptr<Widget>> children;

public:
    ContainerWidget(Window &window, Widget *parent) noexcept : Widget(window, parent)
    {
        _margin = 0.0f;
    }

    ~ContainerWidget() {}

    [[nodiscard]] bool updateConstraints() noexcept override;
    [[nodiscard]] bool updateLayout(hires_utc_clock::time_point display_time_point, bool need_layout) noexcept override;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec window_position) const noexcept override;

    [[nodiscard]] Widget const *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept override;

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual Widget &addWidget(std::unique_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args)
    {
        return static_cast<T &>(addWidget(std::make_unique<T>(window, this, std::forward<Args>(args)...)));
    }

private:
    [[nodiscard]] std::vector<Widget *> childPointers(bool reverse) const noexcept;
};

} // namespace tt
