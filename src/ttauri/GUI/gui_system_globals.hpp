// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "../os_detect.hpp"
#include "../unfair_recursive_mutex.hpp"

namespace tt {

class gui_system;

/** Global mutex for GUI elements, like gui_system, gui_device, Windows and Widgets.
 */
inline unfair_recursive_mutex gui_system_mutex;

}
