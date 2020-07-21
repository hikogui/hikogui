// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"

namespace tt {

struct WidgetPosition {
    int col;
    int row;
    int colspan;
    int rowspan;
    Alignment cellAlignment;

    [[nodiscard]] int firstColumn(int width) noexcept {
        return col >= 0 ? col : width + col;
    }
    [[nodiscard]] int lastColumn(int width) noexcept {
        return firstColumn(width) + colspan - 1;
    }
    [[nodiscard]] int firstRow(int height) noexcept {
        return row >= 0 ? row : height + row;
    }
    [[nodiscard]] int lastRow(int height) noexcept {
        return firstRow(height) + rowspan - 1;
    }
};

class ContainerWidget : public Widget {
protected:
    std::vector<std::unique_ptr<Widget>> children;

public:
    ContainerWidget(Window &window, Widget *parent) noexcept:
        Widget(window, parent, 0.0f, 0.0f) {}

    ~ContainerWidget() {}

    /** Layout children of this widget.
    *
    * Thread safety: locks, must be called from render-thread
    *
    * @param force Force the layout of the widget.
    */
    [[nodiscard]] int layoutChildren(hires_utc_clock::time_point displayTimePoint, bool force) noexcept;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

    [[nodiscard]] Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept override;

    /** Add a widget directly to this widget.
    * Thread safety: locks.
    */
    virtual Widget &addWidget(WidgetPosition position, std::unique_ptr<Widget> childWidget) noexcept;

    virtual WidgetPosition nextPosition() noexcept { return {}; }

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidgetAtPosition(WidgetPosition position, Args &&... args) {
        return static_cast<T &>(
            addWidget(position, std::make_unique<T>(window, this, std::forward<Args>(args)...))
        );
    }

    /** Add a widget directly to this widget.
    *
    * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
    */
    template<typename T, typename... Args>
    T &makeWidget(Args &&... args) {
        return makeWidgetAtPosition<T>(nextPosition(), std::forward<Args>(args)...);
    }

private:
    [[nodiscard]] std::vector<Widget *> childPointers(bool reverse) const noexcept;

};

}
