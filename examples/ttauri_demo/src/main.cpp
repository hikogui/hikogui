
#include "application_controller.hpp"
#include "my_main_window_controller.hpp"
#include "my_preferences_window_controller.hpp"
#include "my_preferences.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/application.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include "ttauri/metadata.hpp"
#include "ttauri/GUI/gfx_system.hpp"
#include "ttauri/audio/audio_system.hpp"
#include <Windows.h>
#include <memory>

int tt_main(int argc, char *argv[], tt::os_handle instance)
{
    // Set the version at the very beginning, because file system paths depend on it.
    auto version = tt::library_metadata();
    version.name = "ttauri-demo";
    version.display_name = "TTauri Demo";
    tt::set_application_metadata(version);

    // Start the logger system, so logging is done asynchronously.
    tt::logger_start();

    // Start the hires_utc_clock subsystem for more accurate time stamps.
    tt::hires_utc_clock::start_subsystem();

    application_controller::global = std::make_shared<::application_controller>();

    my_main_window_controller::global = std::make_shared<::my_main_window_controller>();
    my_preferences_window_controller::global = std::make_shared<::my_preferences_window_controller>();

    my_preferences::global = std::make_unique<my_preferences>(tt::URL::urlFromExecutableDirectory() / "preferences.json");
    my_preferences::global->load();

    tt::audio_system::global().set_delegate(my_preferences_window_controller::global);

    auto app = tt_application(application_controller::global, instance);

    auto r = app.main();

    my_preferences::global->save();
    return r;
}

//extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
