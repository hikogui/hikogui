// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#ifdef _WIN32
#include "VerticalSync_win32.hpp"

namespace TTauri {
using VerticalSync = VerticalSync_win32;
}

#endif
