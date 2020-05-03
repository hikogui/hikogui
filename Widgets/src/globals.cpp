// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/globals.hpp"
#include "TTauri/Widgets/WindowWidget.hpp"
#include "TTauri/GUI/Window.hpp"
#include "TTauri/GUI/Widget.hpp"
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

    Widget::make_unique_WindowWidget = [](Window &window) {
        return std::make_unique<Widgets::WindowWidget>(window);
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
