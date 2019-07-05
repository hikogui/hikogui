// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#ifdef _WIN32

#include "Application_win32.hpp"
#include <memory>

namespace TTauri {

using Application = Application_win32;

}

#endif
