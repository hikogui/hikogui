// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Required/globals.hpp"

namespace TTauri {

RequiredGlobals::RequiredGlobals(std::thread::id main_thread_id, std::string applicationName) :
    main_thread_id(main_thread_id),
    applicationName(std::move(applicationName))
{
    required_assert(Required_globals == nullptr);
    Required_globals = this;
}

RequiredGlobals::~RequiredGlobals()
{
    required_assert(Required_globals == this);
    Required_globals = nullptr;
}

}