// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "gui_system.hpp"

namespace tt {

class gui_system_win32 final : public gui_system {
public:
    gui_system_win32(
        std::unique_ptr<gfx_system> gfx,
        std::unique_ptr<tt::vertical_sync> vertical_sync,
        std::unique_ptr<tt::theme_book> theme_book,
        std::unique_ptr<tt::keyboard_bindings> keyboard_bindings,
        std::weak_ptr<gui_system_delegate> delegate = {});

    void run_from_event_queue(std::function<void()> function) override;

    int loop() override;

    void exit(int exit_code) override;
};

}