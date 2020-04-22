// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/Foundation/globals.hpp"

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
