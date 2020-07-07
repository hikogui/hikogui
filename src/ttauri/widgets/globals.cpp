// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/widgets/globals.hpp"
#include "ttauri/widgets/WindowWidget.hpp"
#include "TTauri/GUI/Window.hpp"
#include "ttauri/widgets/Widget.hpp"
#include "TTauri/GUI/globals.hpp"
#include "ttauri/foundation/globals.hpp"
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
