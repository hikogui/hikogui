// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

// import hikogui;
// import hikogui.crt;

#include "hikogui/module.hpp"
#include "hikogui/task.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto [window, widget] = gui->make_window<window_widget<>>(tr("Radio button example"));
    widget.content().make_widget<label_widget<>>("A1", tr("radio buttons:"));

    /// [Create three radio buttons]
    observer<int> value = 0;

    widget.content().make_widget<radio_button_widget<>>("B1", value, 1, tr("one"));
    widget.content().make_widget<radio_button_widget<>>("B2", value, 2, tr("two"));
    widget.content().make_widget<radio_button_widget<>>("B3", value, 3, tr("three"));
    /// [Create three radio buttons]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
