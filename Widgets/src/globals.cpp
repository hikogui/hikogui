// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::GUI::Widgets {

WidgetsGlobals::WidgetsGlobals()
{
    ttauri_assert(Foundation_globals != nullptr);
    ttauri_assert(Config::Config_globals != nullptr);
    ttauri_assert(GUI::GUI_globals != nullptr);
    ttauri_assert(Widgets_globals == nullptr);
    Widgets_globals = this;
}

WidgetsGlobals::~WidgetsGlobals()
{
    ttauri_assert(Widgets_globals == this);
    Widgets_globals = nullptr;
}

}