
#include "application_controller.hpp"
#include "ttauri/logger.hpp"
#include "ttauri/application.hpp"
#include "ttauri/crt.hpp"
#include "ttauri/hires_utc_clock.hpp"
#include <Windows.h>
#include <memory>

int tt_main(int argc, char *argv[], tt::os_handle instance)
{
    // Start the logger system, so logging is done asynchronously.
    tt::logger_start();

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
