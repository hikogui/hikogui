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
    required_assert(Foundation_globals != nullptr);
    required_assert(Config_globals == nullptr);
    Config_globals = this;
}

ConfigGlobals::~ConfigGlobals()
{
    required_assert(Config_globals == this);
    Config_globals = nullptr;
}

}