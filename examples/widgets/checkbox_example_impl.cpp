// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/task.hpp"
#include "hikogui/widgets/checkbox_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

task<void> checkbox_example(gui_system &gui)
{
    auto window = gui.make_window(tr("Checkbox example"));

    /// [Create a label]
    window->content().make_widget<label_widget>("A1", tr("checkbox:"));
    /// [Create a label]

    /// [Create a checkbox]
    observer<int> value = 0;

    auto &cb = window->content().make_widget<checkbox_widget>("B1", value, 1, 2);
    cb.on_label = tr("on");
    cb.off_label = tr("off");
    cb.other_label = tr("other");
    /// [Create a checkbox]

    co_await window->closing;
}

int hi_main(int argc, char* argv[])
{
    auto gui = gui_system::make_unique();
    checkbox_example(*gui);
    return loop::main().resume();
}
