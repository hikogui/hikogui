
#pragma once

#include <compare> // XXX #619
#include <cstddef> // XXX #619
#include <memory> // XXX #619
#include <chrono> // XXX #619
#include "async_task.hpp" // export
#include "awaitable_stop_token_intf.hpp" // export
#include "awaitable_stop_token_impl.hpp" // export
#include "awaitable_timer_intf.hpp" // export
#include "awaitable_timer_impl.hpp" // export
#include "awaitable.hpp" // export
#include "function_timer.hpp" // export
#include "loop_win32_intf.hpp" // export
#include "notifier.hpp" // export
#include "progress.hpp" // export
#include "socket_event.hpp" // export
#include "task_controller.hpp" // export
#include "task.hpp" // export
#include "when_any.hpp" // export

/** @module hikogui.dispatch
 * @brief Asynchronous dispatching and co-routine tasks.
 *
 * The Dispatch Loop
 * -----------------
 * Each thread has a local dispatch loop, which can be accessed using
 * the static function `hi::loop::local()`, which will return a `hi::loop`
 * instance.
 *
 * There are also two well-known loops which can be accessed using the
 * static functions `hi::loop::main()` and `hi::loop::timer()`.
 * 
 * The main-loop is used to handle events from the GUI, Audio-control and
 * network. The main-loop is latency sensitive and any event should be
 * handled quickly.
 *
 * The timer-loop is less latency sensitive and can be used for slower
 * maintenace tasks, such as: high-resolution clock synchronization and logging
 * of telemetry.
 *
 * To "pump" the events you can use either `hi::loop::resume_once()` which
 * will handle events for one iteration; or `hi::loop::resume()` which
 * will handle events until exit is requested, or until there are no more
 * event handlers registered.
 *
 * Currently you will need to use `hi::loop::resume()` to handle the GUI
 * system. One windows a complex setup of threads and priorities is needed
 * to properly handle redraws, windows-events and networking on the same
 * thread which is setup by `hi::loop:resume()`.
 * 
 */
hi_export_module(hikogui.dispatch);
