// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/selection_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto window = gui->make_window(tr("Selection box example"));
    window->content().make_widget<label_widget>("A1", tr("Selection Box"), alignment::middle_center());

    /// [Create selection]
    auto option_list = std::vector<std::pair<int, label>>{{1, tr("one")}, {2, tr("two")}, {3, tr("three")}};

    observer<int> value = 0;
    window->content().make_widget<selection_widget>("A2", value, option_list);
    /// [Create selection]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
