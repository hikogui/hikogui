// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Audio/globals.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri::Audio {

AudioGlobals::AudioGlobals()
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals != nullptr);
    required_assert(Diagnostic_globals != nullptr);
    required_assert(Foundation_globals != nullptr);
    required_assert(Audio_globals == nullptr);
    Audio_globals = this;

}

AudioGlobals::~AudioGlobals()
{

    required_assert(Audio_globals == this);
    Audio_globals = nullptr;
}

}