// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/hikogui.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    set_application_name("Radio button example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto widget = std::make_unique<window_widget>(txt("Radio button example"));
    widget->content().emplace<label_widget>("A1", txt("radio buttons:"));

    /// [Create three radio buttons]
    observer<int> value = 0;

    widget->content().emplace<radio_button_with_label_widget>("B1", value, 1, txt("one"));
    widget->content().emplace<radio_button_with_label_widget>("B2", value, 2, txt("two"));
    widget->content().emplace<radio_button_with_label_widget>("B3", value, 3, txt("three"));
    /// [Create three radio buttons]

    auto window = std::make_unique<gui_window>(std::move(widget));

    auto close_cbt = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
