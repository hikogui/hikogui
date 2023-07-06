// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/crt.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    set_application_name("Tab example");
    set_application_vendor("HikoGUI");
    set_application_version({1, 0, 0});

    auto gui = gui_system::make_unique();
    auto [window, widget] = gui->make_window<window_widget>(tr("tab example"));

    observer<int> value = 0;

    /// [Create three tabs]
    auto& tab_view = widget.content().make_widget<tab_widget>("A1", value);
    tab_view.make_widget<label_widget>(0, tr("one"), alignment::middle_center());
    tab_view.make_widget<label_widget>(1, tr("two"), alignment::middle_center());
    tab_view.make_widget<label_widget>(2, tr("three"), alignment::middle_center());
    /// [Create three tabs]

    /// [Create three toolbar tab buttons]
    widget.toolbar().make_widget<toolbar_tab_button_widget>(value, 0, tr("one"));
    widget.toolbar().make_widget<toolbar_tab_button_widget>(value, 1, tr("two"));
    widget.toolbar().make_widget<toolbar_tab_button_widget>(value, 2, tr("three"));
    /// [Create three toolbar tab buttons]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
