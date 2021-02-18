
#include "ApplicationController.hpp"
#include "ttauri/application.hpp"
#include "ttauri/crt.hpp"
#include <Windows.h>
#include <memory>

int tt_main(std::vector<std::string> arguments, tt::os_handle instance)
{
    auto application_controller = std::make_shared<ttauri_demo::ApplicationController>();
    ttauri_demo::ApplicationController::global = application_controller;

    auto app = tt_application(application_controller, std::move(arguments), instance);
    return app.main();
}

//extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}