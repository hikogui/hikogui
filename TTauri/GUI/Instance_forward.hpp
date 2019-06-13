// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::GUI {

#ifdef _WIN32
    class Instance_vulkan_win32;
    using Instance = Instance_vulkan_win32;
#endif

}