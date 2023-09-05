// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    set_application_name("Toggle example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto widget = std::make_unique<window_widget>(txt("Toggle example"));
    widget->content().make_widget<label_widget>("A1", txt("toggle:"));

    /// [Create a toggle]
    observer<int> value = 0;

    auto& tb = widget->content().make_widget<toggle_widget>("B1", value, 1, 2);
    tb.on_label = txt("on");
    tb.off_label = txt("off");
    tb.other_label = txt("other");
    /// [Create a toggle]

    auto window = std::make_unique<gui_window>(std::move(widget));

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
