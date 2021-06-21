// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {
class gui_system;

class gui_system_delegate {
public:
    virtual void last_window_closed(gui_system &self) = 0;
};

}
