// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

task<void> checkbox_example(gui_system &gui)
{
    auto [window, widget] = gui.make_window<window_widget>(tr("Checkbox example"));

    /// [Create a label]
    widget.content().make_widget<label_widget>("A1", tr("checkbox:"));
    /// [Create a label]

    /// [Create a checkbox]
    observer<int> value = 0;

    auto& cb = widget.content().make_widget<checkbox_widget>("B1", value, 1, 2);
    cb.on_label = tr("on");
    cb.off_label = tr("off");
    cb.other_label = tr("other");
    /// [Create a checkbox]

    co_await window->closing;
}

int hi_main(int argc, char* argv[])
{
    set_application_name("Checkbox example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto gui = gui_system::make_unique();
    checkbox_example(*gui);
    return loop::main().resume();
}
