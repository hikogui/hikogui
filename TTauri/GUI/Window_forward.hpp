// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace TTauri::GUI {

#ifdef _WIN32
class Window_vulkan_win32;
using Window = Window_vulkan_win32;
#endif

}