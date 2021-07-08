// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

int tt_main(int argc, char *argv[])
{
    auto &window = gui_system::global().make_window(l10n("Radio button example"));
    window.make_widget<label_widget>("A1", l10n("radio buttons:"));

/// [Create three radio buttons]
    observable<int> value = 0;

    window.make_widget<radio_button_widget>("B1", l10n("one"), value, 1);
    window.make_widget<radio_button_widget>("B2", l10n("two"), value, 2);
    window.make_widget<radio_button_widget>("B3", l10n("three"), value, 3);
/// [Create three radio buttons]

    return gui_system::global().loop();
}
