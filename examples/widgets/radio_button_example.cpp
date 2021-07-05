
#include "ttauri/logger.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include "ttauri/metadata.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/widgets/widgets.hpp"
#include <Windows.h>
#include <memory>

using namespace tt;

int tt_main(int argc, char *argv[])
{
    set_application_metadata("radio-button-example");


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

