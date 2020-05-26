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

namespace TTauri::GUI::Widgets {

void startup()
{
    if (startupCount.fetch_add(1) != 0) {
        // The library has already been initialized.
        return;
    }

    TTauri::startup();
    TTauri::GUI::startup();
    LOG_INFO("TTauri::GUI::Widgets startup");

    Widget_delete = [](auto *self) {
        return delete self;
    };

    WindowWidget_makeUnique = [](auto &window) {
        return std::unique_ptr<Widgets::WindowWidget,WidgetDeleter>{ new Widgets::WindowWidget(window) };
    };

    Widget_needs = [](let &self, auto displayTimePoint) {
        return self.needs(displayTimePoint);
    };

    Widget_layout = [](auto &self, auto displayTimePoint) {
        self.layout(displayTimePoint);
    };

    Widget_draw = [](auto &self, let &drawContext, auto displayTimePoint) {
        self.draw(drawContext, displayTimePoint);
    };

    Widget_layoutChildren = [](auto &self, auto displayTimePoint, auto force) {
        return self.layoutChildren(displayTimePoint, force);
    };

    Widget_getNextKeyboardWidget = [](let &self) {
        return self.nextKeyboardWidget;
    };

    Widget_getPreviousKeyboardWidget = [](let &self) {
        return self.prevKeyboardWidget;
    };

    Widget_acceptsFocus = [](let &self) {
        return self.acceptsFocus();
    };

    Widget_handleMouseEvent = [](auto &self, let &event) {
        self.handleMouseEvent(event);
    };

    Widget_handleKeyboardEvent = [](auto &self, let &event) {
        self.handleKeyboardEvent(event);
    };

    Widget_getWindowOffset = [](let &self) {
        return self.offsetFromWindow.load(std::memory_order::memory_order_relaxed);
    };

    Widget_hitBoxTest = [](let &self, auto position) {
        return self.hitBoxTest(position);
    };

    Widget_getBox = [](let &self) -> BoxModel const& {
        return self.box;
    };
}

void shutdown()
{
    if (startupCount.fetch_sub(1) != 1) {
        // This is not the last instantiation.
        return;
    }
    LOG_INFO("TTauri::GUI::Widgets shutdown");

    TTauri::GUI::shutdown();
    TTauri::shutdown();
}

}
