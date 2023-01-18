// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/module.hpp"
#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/tab_widget.hpp"
#include "hikogui/widgets/toolbar_tab_button_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto window = gui->make_window(tr("tab example"));

    observer<int> value = 0;

    /// [Create three tabs]
    auto& tab_view = window->content().make_widget<tab_widget>("A1", value);
    tab_view.make_widget<label_widget>(0, tr("one"), alignment::middle_center());
    tab_view.make_widget<label_widget>(1, tr("two"), alignment::middle_center());
    tab_view.make_widget<label_widget>(2, tr("three"), alignment::middle_center());
    /// [Create three tabs]

    /// [Create three toolbar tab buttons]
    window->toolbar().make_widget<toolbar_tab_button_widget>(value, 0, tr("one"));
    window->toolbar().make_widget<toolbar_tab_button_widget>(value, 1, tr("two"));
    window->toolbar().make_widget<toolbar_tab_button_widget>(value, 2, tr("three"));
    /// [Create three toolbar tab buttons]

    auto close_cb = window->closing.subscribe(
        [&] {
            window.reset();
        },
        hi::callback_flags::main);
    return loop::main().resume();
}
