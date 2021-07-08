// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/widgets/selection_widget.hpp"
#include "ttauri/crt.hpp"

using namespace tt;

int tt_main(int argc, char *argv[])
{
    auto &window = gui_system::global().make_window(l10n("Radio button example"));
    window.make_widget<label_widget>("A1", l10n("Selection Box"), alignment::middle_center);

    /// [Create selection]
    auto option_list = std::vector<std::pair<int,label>>{
        {1, l10n("one")},
        {2, l10n("two")},
        {3, l10n("three")}
    };

    observable<int> value = 0;
    window.make_widget<selection_widget>("A2", option_list, value);
    /// [Create selection]

    return gui_system::global().loop();
}
