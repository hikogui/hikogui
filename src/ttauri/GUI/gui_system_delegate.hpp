// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {
class gui_system;

class gui_system_delegate {
public:
    virtual void last_window_closed(gui_system &self) = 0;
};

}
