// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    set_application_name("Selection example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto [window, widget] = make_unique_window<window_widget>(txt("Selection box example"));
    widget.content().make_widget<label_widget>("A1", txt("Selection Box"), alignment::middle_center());

    /// [Create selection]
    auto option_list = std::vector<std::pair<int, label>>{{1, txt("one")}, {2, txt("two")}, {3, txt("three")}};

    observer<int> value = 0;
    widget.content().make_widget<selection_widget>("A2", value, option_list);
    /// [Create selection]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
