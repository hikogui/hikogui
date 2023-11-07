// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file terminate.hpp Utilities for throwing exceptions and terminating the application.
 */

module;
#include "../macros.hpp"

#include <exception>
#include <stdexcept>
#include <atomic>
#include <bit>
#include <format>
#include <iostream>

export module hikogui_crt_terminate;
import hikogui_char_maps; // XXX #616
import hikogui_console;
import hikogui_telemetry;
import hikogui_utility;

export namespace hi { inline namespace v1 {

/** The old terminate handler.
 *
 * This is the handler returned by `std::set_terminate()`.
 */
std::terminate_handler old_terminate_handler;

/** The HikoGUI terminate handler.
 *
 * This handler will print an error message on the console or pop-up a dialogue box.
 *
 * @note Use `hi_set_terminate_message()` to set a message.
 */
void terminate_handler() noexcept{
    using namespace std::literals;

    log_global.flush();

    auto title = std::string{};
    auto message = std::string{};

    hilet ep = std::current_exception();
    if (ep) {
        try {
            std::rethrow_exception(ep);

        } catch (std::exception const& e) {
            title = "Unhandled std::exception."s;
            message = e.what();

        } catch (...) {
            title = "Unhandled unknown exception."s;
            message = "<no data>"s;
        }

        std::cerr << std::format("{}\n{}\n", title, message);

    } else {
        title = "Abnormal termination."s;
        message = debug_message.exchange(nullptr, std::memory_order::relaxed);
    }

    dialog_ok(title, message);

    return old_terminate_handler();
}

}} // namespace hi::v1
