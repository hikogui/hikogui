// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Draw/globals.hpp"
#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::Draw {

DrawGlobals::DrawGlobals()
{
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