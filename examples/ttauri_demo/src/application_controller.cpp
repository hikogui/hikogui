// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "application_controller.hpp"
#include "application_preferences.hpp"
#include "ttauri/log_level.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/application.hpp"
#include "ttauri/CommandLineParser.hpp"

namespace demo {

std::optional<int> application_controller::main(tt::application& self)
{
    application_preferences::global = std::make_unique<application_preferences>(tt::URL::urlFromExecutableDirectory() / "preferences.json");
    application_preferences::global->load();

    tt_axiom(tt::gui_system::global != nullptr);
    tt::gui_system::global->make_window(main_window_controller, tt::label{ tt::URL{"resource:ttauri_demo.png"}, tt::l10n("ttauri_demo") });
    return {};
}

void application_controller::last_window_closed(tt::gui_system& self)
{
    application_preferences::global->save();
    tt::application::global->exit();
}

void application_controller::audio_device_list_changed(tt::audio_system& self)
{
    return preferences_controller->audio_device_list_changed(self);
}

}
