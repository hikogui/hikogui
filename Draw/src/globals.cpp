// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Draw/globals.hpp"
#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri::Draw {

DrawGlobals::DrawGlobals()
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals != nullptr);
    required_assert(Diagnostic_globals != nullptr);
    required_assert(Foundation_globals != nullptr);
    required_assert(Config::Config_globals != nullptr);
    required_assert(Draw_globals == nullptr);
    Draw_globals = this;
}

DrawGlobals::~DrawGlobals()
{
    required_assert(Draw_globals == this);
    Draw_globals = nullptr;
}

}