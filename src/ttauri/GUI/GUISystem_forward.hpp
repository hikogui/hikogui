// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../os_detect.hpp"
#include "../unfair_recursive_mutex.hpp"

namespace tt {

class GUISystem_base;

/** Global mutex for GUI elements, like GUISystem, GUIDevice, Windows and Widgets.
 */
inline unfair_recursive_mutex GUISystem_mutex;

}
