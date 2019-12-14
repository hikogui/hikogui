// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Config/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/globals.hpp"

namespace TTauri::Config {

ConfigGlobals::ConfigGlobals()
{
    ttauri_assert(Foundation_globals != nullptr);
    ttauri_assert(Config_globals == nullptr);
    Config_globals = this;
}

ConfigGlobals::~ConfigGlobals()
{
    ttauri_assert(Config_globals == this);
    Config_globals = nullptr;
}

}