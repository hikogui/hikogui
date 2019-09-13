// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Widgets/globals.hpp"
#include "TTauri/GUI/globals.hpp"
#include "TTauri/Draw/globals.hpp"
#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri::GUI::Widgets {

WidgetsGlobals::WidgetsGlobals()
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals != nullptr);
    required_assert(Diagnostic_globals != nullptr);
    required_assert(Foundation_globals != nullptr);
    required_assert(Config::Config_globals != nullptr);
    required_assert(Draw::Draw_globals != nullptr);
    required_assert(GUI::GUI_globals != nullptr);
    required_assert(Widgets_globals == nullptr);
    Widgets_globals = this;
}

WidgetsGlobals::~WidgetsGlobals()
{
    required_assert(Widgets_globals == this);
    Widgets_globals = nullptr;
}

}