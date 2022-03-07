// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/radio_button_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

int tt_main(int argc, char *argv[])
{
    auto gui = gui_system::make_unique();
    auto window = gui->make_window(l10n("Radio button example"));
    window->content().make_widget<label_widget>("A1", l10n("radio buttons:"));

/// [Create three radio buttons]
    observable<int> value = 0;

    window->content().make_widget<radio_button_widget>("B1", l10n("one"), value, 1);
    window->content().make_widget<radio_button_widget>("B2", l10n("two"), value, 2);
    window->content().make_widget<radio_button_widget>("B3", l10n("three"), value, 3);
/// [Create three radio buttons]

    auto close_cb = window->subscribe_close([&]{ window.reset(); });
    return gui->loop();
}
