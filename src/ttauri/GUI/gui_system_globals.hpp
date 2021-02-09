// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../os_detect.hpp"
#include "../unfair_recursive_mutex.hpp"

namespace tt {

class gui_system;

/** Global mutex for GUI elements, like gui_system, gui_device, Windows and Widgets.
 */
inline unfair_recursive_mutex gui_system_mutex;

}
