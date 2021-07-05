
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
    set_application_metadata("checkbox-example");


    auto &window = gui_system::global().make_window(l10n("Checkbox example"));
    window.make_widget<label_widget>("A1", l10n("checkbox:"));

/// [Create a checkbox]
    observable<int> value = 0;

    auto &cb = window.make_widget<checkbox_widget>("B1", value, 1, 2);
    cb.on_label = l10n("on");
    cb.off_label = l10n("off");
    cb.other_label = l10n("other");
/// [Create a checkbox]

    return gui_system::global().loop();
}
