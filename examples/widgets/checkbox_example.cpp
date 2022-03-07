// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/checkbox_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

gui_task<void> checkbox_example(gui_system &gui)
{
    auto window = gui.make_window(l10n("Checkbox example"));

    /// [Create a label]
    window.content().make_widget<label_widget>("A1", l10n("checkbox:"));
    /// [Create a label]

    /// [Create a checkbox]
    observable<int> value = 0;

    auto &cb = window.content().make_widget<checkbox_widget>("B1", value, 1, 2);
    cb.on_label = l10n("on");
    cb.off_label = l10n("off");
    cb.other_label = l10n("other");
    /// [Create a checkbox]

    co_await window.await_close();
}

int tt_main(int argc, char* argv[])
{
    auto gui = gui_system::make_unique();

    checkbox_example(*gui);
    
    return gui->loop();
}
