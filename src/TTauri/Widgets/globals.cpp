// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/globals.hpp"
#include "TTauri/Widgets/WindowWidget.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Widget_forward.hpp"
#include "TTauri/Widgets/Widget.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include <memory>

namespace tt {

/** Reference counter to determine the amount of startup/shutdowns.
*/
static std::atomic<uint64_t> startupCount = 0;


void widgets_startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    foundation_startup();
    gui_startup();
    LOG_INFO("Widgets startup");

    Widget_delete = [](auto *self) {
        return delete self;
    };

    WindowWidget_makeUnique = [](auto &window) {
        return std::unique_ptr<WindowWidget,WidgetDeleter>{ new WindowWidget(window) };
    };

    Widget_needs = [](auto &self, auto displayTimePoint) {
        return self.needs(displayTimePoint);
    };

    Widget_layout = [](auto &self, auto displayTimePoint) {
        self.layout(displayTimePoint);
    };

    Widget_draw = [](auto &self, ttlet &drawContext, auto displayTimePoint) {
        self.draw(drawContext, displayTimePoint);
    };

    Widget_layoutChildren = [](auto &self, auto displayTimePoint, auto force) {
        return self.layoutChildren(displayTimePoint, force);
    };

    Widget_getNextKeyboardWidget = [](ttlet &self) {
        return self.nextKeyboardWidget;
    };

    Widget_getPreviousKeyboardWidget = [](ttlet &self) {
        return self.prevKeyboardWidget;
    };

    Widget_acceptsFocus = [](ttlet &self) {
        return self.acceptsFocus();
    };

    Widget_handleMouseEvent = [](auto &self, ttlet &event) {
        self.handleMouseEvent(event);
    };

    Widget_handleKeyboardEvent = [](auto &self, ttlet &event) {
        self.handleKeyboardEvent(event);
    };

    Widget_getWindowOffset = [](ttlet &self) {
        return self.offsetFromWindow();
    };

    Widget_hitBoxTest = [](ttlet &self, auto position) {
        return self.hitBoxTest(position);
    };

    Widget_getLeft = [](ttlet &self) -> rhea::variable const & {
        return self.left;
    };

    Widget_getBottom = [](ttlet &self) -> rhea::variable const & {
        return self.bottom;
    };

    Widget_getWidth = [](ttlet &self) -> rhea::variable const & {
        return self.width;
    };

    Widget_getHeight = [](ttlet &self) -> rhea::variable const & {
        return self.height;
    };

    Widget_getRight = [](ttlet &self) -> rhea::linear_expression const & {
        return self.right;
    };

    Widget_getTop = [](ttlet &self) -> rhea::linear_expression const & {
        return self.right;
    };
}

void widgets_shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("Widgets shutdown");

    gui_shutdown();
    foundation_shutdown();
}

}
