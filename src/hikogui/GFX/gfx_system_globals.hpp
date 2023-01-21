// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../utility/module.hpp"

namespace hi::inline v1 {

class gfx_system;

/** Global mutex for GUI elements, like gfx_system, gfx_device, Windows and Widgets.
 */
inline unfair_recursive_mutex gfx_system_mutex;

} // namespace hi::inline v1
