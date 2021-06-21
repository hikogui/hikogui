// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace tt {
class gfx_system;

class gfx_system_delegate {
public:
    virtual void last_window_closed(gfx_system &self) = 0;
};

}
