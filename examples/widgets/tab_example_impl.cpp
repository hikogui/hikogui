// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/hikogui.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    set_application_name("Tab example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto widget = std::make_unique<window_widget>(txt("tab example"));

    observer<int> value = 0;

    /// [Create three tabs]
    auto& tab_view = widget->content().emplace<tab_widget>("A1", value);
    tab_view.emplace<label_widget>(0, txt("one"), alignment::middle_center());
    tab_view.emplace<label_widget>(1, txt("two"), alignment::middle_center());
    tab_view.emplace<label_widget>(2, txt("three"), alignment::middle_center());
    /// [Create three tabs]

    /// [Create three toolbar tab buttons]
    widget->toolbar().emplace<toolbar_tab_button_widget>(value, 0, txt("one"));
    widget->toolbar().emplace<toolbar_tab_button_widget>(value, 1, txt("two"));
    widget->toolbar().emplace<toolbar_tab_button_widget>(value, 2, txt("three"));
    /// [Create three toolbar tab buttons]

    auto window = std::make_unique<gui_window>(std::move(widget));

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
