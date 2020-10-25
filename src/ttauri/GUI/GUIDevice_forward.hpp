// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../unfair_recursive_mutex.hpp"

namespace tt {
class GUIDevice_vulkan;
using GUIDevice = GUIDevice_vulkan;

/** Global mutex for GUI elements, like GUISystem, GUIDevice, Windows and Widgets.
 */
inline unfair_recursive_mutex GUISystem_mutex;

}

