
#include "application_controller.hpp"
#include "ttauri/application.hpp"
#include "ttauri/crt.hpp"
#include <Windows.h>
#include <memory>

int tt_main(std::vector<std::string> arguments, tt::os_handle instance)
{
    auto application_controller = std::make_shared<demo::application_controller>();
    demo::application_controller::global = application_controller;

    auto app = tt_application(application_controller, std::move(arguments), instance);
    return app.main();
}

//extern "C" const char *__asan_default_options() {
//    return "help=1:log_path=asan.log";
//}
