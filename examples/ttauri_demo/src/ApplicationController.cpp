// Copyright 2020 Pokitec
// All rights reserved.

#include "ApplicationController.hpp"
#include "ApplicationPreferences.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/application.hpp"
#include "ttauri/CommandLineParser.hpp"

namespace ttauri_demo {

using namespace tt;

std::string ApplicationController::application_name(application &self) const noexcept {
    return "TTauri Demo";
}

datum ApplicationController::configuration(application &self, std::vector<std::string> arguments) const noexcept {
    auto parser = CommandLineParser(
        "TTauri Demo application."
    );
    parser.add(
        "help"s,
        datum_type_t::Boolean,
        "This help message"s
    );
    parser.add(
        "log-level"s,
        datum_type_t::Integer,
        "Set the log level, possible values 'debug', 'info', 'audit', 'warning', 'error', 'critical' or 'fatal'."s,
        command_line_argument_to_log_level
    );

    auto default_configuration = datum{ datum::map{} };
    default_configuration["help"] = false;
    default_configuration["log-level"] = static_cast<uint8_t>(log_level::Debug);

    ttlet command_line_configuration = parser.parse(arguments);
    auto configuration = deep_merge(default_configuration, command_line_configuration);

    if (parser.has_error() || configuration["help"]) {
        parser.print_help();
        std::exit(parser.has_error() ? 2 : 0);
    }

    tt_log_info("Configuration {}", configuration);
    return configuration;
}

std::optional<int> ApplicationController::main(application &self)
{
    ApplicationPreferences::global = std::make_unique<ApplicationPreferences>(URL::urlFromExecutableDirectory() / "preferences.json");
    ApplicationPreferences::global->load();

    tt_axiom(gui_system::global != nullptr);
    gui_system::global->make_window(main_window_controller, label{URL{"resource:ttauri_demo.png"}, l10n("ttauri_demo")});
    return {};
}

void ApplicationController::last_window_closed(gui_system &self)
{
    ApplicationPreferences::global->save();
    application::global->exit();
}

void ApplicationController::audio_device_list_changed(tt::audio_system &self)
{
    return preferences_controller->audio_device_list_changed(self);
}

}
