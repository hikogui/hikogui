// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "application_controller.hpp"
#include "my_preferences.hpp"
#include "my_main_window_controller.hpp"
#include "ttauri/log_level.hpp"
#include "ttauri/GUI/gfx_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/application.hpp"
#include "ttauri/CommandLineParser.hpp"

std::optional<int> application_controller::main(tt::application& self)
{
    tt_axiom(tt::gfx_system::global != nullptr);
    tt::gfx_system::global->make_window(my_main_window_controller::global, tt::label{ tt::URL{"resource:ttauri_demo.png"}, tt::l10n("ttauri_demo") });
    return {};
}

void application_controller::last_window_closed(tt::gfx_system& self)
{
    tt::application::global->exit();
}

