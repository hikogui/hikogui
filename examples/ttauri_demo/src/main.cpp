
#include "application_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/application.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include "ttauri/metadata.hpp"
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

    auto application_controller = std::make_shared<demo::application_controller>();
    demo::application_controller::global = application_controller;

    tt_log_info("compatible clock: {}", tt::format_iso8601(tt::hires_utc_clock::now()));


    tt_log_info("leap second aware clock: {}", tt::format_iso8601(tt::hires_utc_clock::now()));

    auto app = tt_application(application_controller, argc, argv, instance);
    return app.main();
}

//extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
