// Copyright Take Vos 2020-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "application_controller.hpp"
#include "application_preferences.hpp"
#include "ttauri/system_status.hpp"
#include "ttauri/GUI/gui_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include "ttauri/application.hpp"
#include "ttauri/CommandLineParser.hpp"

#include "demo_version.hpp"

namespace demo {

tt::version application_controller::application_version(tt::application &self) const noexcept {
    return demo_version;
}

tt::datum application_controller::configuration(tt::application &self, std::vector<std::string> arguments) const noexcept {
    using namespace std::literals;

    auto parser = tt::CommandLineParser(
        "TTauri Demo application."
    );
    parser.add(
        "help"s,
        tt::datum_type_t::Boolean,
        "This help message"s
    );
    parser.add(
        "log-level"s,
        tt::datum_type_t::Integer,
        "Set the log level, possible values 'debug', 'info', 'audit', 'warning', 'error', 'critical' or 'fatal'."s,
        tt::command_line_argument_to_log_level
    );

    auto default_configuration = tt::datum{ tt::datum::map{} };
    default_configuration["help"] = false;
    default_configuration["log-level"] = static_cast<int>(make_log_level(tt::log_level::debug));

    ttlet command_line_configuration = parser.parse(arguments);
    auto configuration = deep_merge(default_configuration, command_line_configuration);

    if (parser.has_error() || configuration["help"]) {
        parser.print_help();
        std::exit(parser.has_error() ? 2 : 0);
    }

    tt_log_info("Configuration {}", configuration);
    return configuration;
}

std::optional<int> application_controller::main(tt::application &self)
{
    application_preferences::global = std::make_unique<application_preferences>(tt::URL::urlFromExecutableDirectory() / "preferences.json");
    application_preferences::global->load();

    tt_axiom(tt::gui_system::global != nullptr);
    tt::gui_system::global->make_window(main_window_controller, tt::label{tt::URL{"resource:ttauri_demo.png"}, tt::l10n("ttauri_demo")});
    return {};
}

void application_controller::last_window_closed(tt::gui_system &self)
{
    application_preferences::global->save();
    tt::application::global->exit();
}

void application_controller::audio_device_list_changed(tt::audio_system &self)
{
    return preferences_controller->audio_device_list_changed(self);
}

}
