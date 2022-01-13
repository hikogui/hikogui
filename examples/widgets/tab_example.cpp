// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/tab_widget.hpp"
#include "ttauri/widgets/toolbar_tab_button_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

int tt_main(int argc, char* argv[])
{
    auto gui = gui_system::make_unique();
    auto& window = gui->make_window(l10n("tab example"));

    observable<int> value = 0;

    /// [Create three tabs]
    auto &tab_view = window.content().make_widget<tab_widget>("A1", value);
    auto &l1 = tab_view.make_widget<label_widget>(0, l10n("one"));
    auto &l2 = tab_view.make_widget<label_widget>(1, l10n("two"));
    auto &l3 = tab_view.make_widget<label_widget>(2, l10n("three"));
    /// [Create three tabs]

    l1.alignment = alignment::middle_center();
    l2.alignment = alignment::middle_center();
    l3.alignment = alignment::middle_center();

    /// [Create three toolbar tab buttons]
    window.toolbar().make_widget<toolbar_tab_button_widget>(l10n("one"), value, 0);
    window.toolbar().make_widget<toolbar_tab_button_widget>(l10n("two"), value, 1);
    window.toolbar().make_widget<toolbar_tab_button_widget>(l10n("three"), value, 2);
    /// [Create three toolbar tab buttons]

    return gui->loop();
}

