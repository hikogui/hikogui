// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "Widget.hpp"
#include "ContainerWidgetDelegate.hpp"
#include "../cell_address.hpp"

namespace tt {

class ContainerWidget : public Widget {
protected:
    std::vector<std::unique_ptr<Widget>> children;
    cell_address current_address = "L0T0"_ca;

    ContainerWidgetDelegate *delegate = nullptr;

public:
    ContainerWidget(Window &window, Widget *parent, ContainerWidgetDelegate *delegate=nullptr) noexcept :
        Widget(window, parent), delegate(delegate)
    {
        if (delegate) {
            delegate->openingContainerWidget(*this);
        }
    }

    ~ContainerWidget() {
        if (delegate) {
            delegate->closingContainerWidget(*this);
        }
    }

    [[nodiscard]] WidgetUpdateResult updateConstraints() noexcept override;
    [[nodiscard]] WidgetUpdateResult
    updateLayout(hires_utc_clock::time_point displayTimePoint, bool forceLayout) noexcept override;

    void draw(DrawContext const &drawContext, hires_utc_clock::time_point displayTimePoint) noexcept override;

    [[nodiscard]] HitBox hitBoxTest(vec position) const noexcept override;

    [[nodiscard]] Widget *nextKeyboardWidget(Widget const *currentKeyboardWidget, bool reverse) const noexcept override;

    /** Add a widget directly to this widget.
     * Thread safety: locks.
     */
    virtual Widget &addWidget(cell_address address, std::unique_ptr<Widget> childWidget) noexcept;

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, typename... Args>
    T &makeWidgetAtAddress(cell_address address, Args &&... args)
    {
        return static_cast<T &>(addWidget(address, std::make_unique<T>(window, this, std::forward<Args>(args)...)));
    }

    /** Add a widget directly to this widget.
     *
     * Thread safety: modifies atomic. calls addWidget() and addWidgetDirectly()
     */
    template<typename T, cell_address CellAddress, typename... Args>
    T &makeWidget(Args &&... args)
    {
        return makeWidgetAtAddress<T>(CellAddress, std::forward<Args>(args)...);
    }

private:
    [[nodiscard]] std::vector<Widget *> childPointers(bool reverse) const noexcept;
};

} // namespace tt
