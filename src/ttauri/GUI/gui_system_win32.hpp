// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_system.hpp"

namespace tt {

class gui_system_win32 final : public gui_system {
public:
    gui_system_win32();

    void run_from_event_queue(std::function<void()> function) override;

    int loop() override;

    void exit(int exit_code) override;
};

}