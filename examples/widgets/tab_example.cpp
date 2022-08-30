// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hikogui/GUI/gui_system.hpp"
#include "hikogui/widgets/tab_widget.hpp"
#include "hikogui/widgets/toolbar_tab_button_widget.hpp"
#include "hikogui/crt.hpp"
#include "hikogui/loop.hpp"

using namespace hi;

int hi_main(int argc, char* argv[])
{
    auto gui = gui_system::make_unique();
    auto window = gui->make_window(tr("tab example"));

    observer<int> value = 0;

    /// [Create three tabs]
    auto &tab_view = window->content().make_widget<tab_widget>("A1", value);
    auto &l1 = tab_view.make_widget<label_widget>(0, tr("one"));
    auto &l2 = tab_view.make_widget<label_widget>(1, tr("two"));
    auto &l3 = tab_view.make_widget<label_widget>(2, tr("three"));
    /// [Create three tabs]

    l1.alignment = alignment::middle_center();
    l2.alignment = alignment::middle_center();
    l3.alignment = alignment::middle_center();

    /// [Create three toolbar tab buttons]
    window->toolbar().make_widget<toolbar_tab_button_widget>(tr("one"), value, 0);
    window->toolbar().make_widget<toolbar_tab_button_widget>(tr("two"), value, 1);
    window->toolbar().make_widget<toolbar_tab_button_widget>(tr("three"), value, 2);
    /// [Create three toolbar tab buttons]

    auto close_cb = window->closing.subscribe(hi::callback_flags::main, [&] {
        window.reset();
    });
    return loop::main().resume();
}

