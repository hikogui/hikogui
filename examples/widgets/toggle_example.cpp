// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/toggle_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto window = gui->make_window(tr("Toggle example"));
    window->content().make_widget<label_widget>("A1", tr("toggle:"));

    /// [Create a toggle]
    observer<int> value = 0;

    auto& tb = window->content().make_widget<toggle_widget>("B1", value, 1, 2);
    tb.on_label = tr("on");
    tb.off_label = tr("off");
    tb.other_label = tr("other");
    /// [Create a toggle]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
