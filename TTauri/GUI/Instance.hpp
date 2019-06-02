// Copyright 2019 Pokitec
// All rights reserved.

#pragma once
#include <memory>

#ifdef _WIN32
#include "Instance_vulkan_win32.hpp"

namespace TTauri::GUI {

using Instance = Instance_vulkan_win32;

}
#endif

namespace TTauri::GUI {

extern std::unique_ptr<Instance> instance;

}
