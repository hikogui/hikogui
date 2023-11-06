
module;

#include <compare> // XXX #619
#include <cstddef> // XXX #619
#include <memory> // XXX #619
#include <chrono> // XXX #619

export module hikogui_dispatch;
export import : awaitable_timer_impl;
export import : awaitable_timer_intf;
export import : loop_impl;
export import : loop_intf;
export import : notifier;
export import : scoped_task;
export import : when_any;
export import hikogui_dispatch_function_timer;
export import hikogui_dispatch_socket_event;
