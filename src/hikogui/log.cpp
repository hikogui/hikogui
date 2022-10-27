// Copyright Take Vos 2019, 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "log.hpp"
#include "trace.hpp"
#include "utility.hpp"
#include "strings.hpp"
#include "thread.hpp"
#include "console.hpp"
#include <format>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>
#include <thread>

namespace hi::inline v1 {

void log::log_thread_main(std::stop_token stop_token) noexcept
{
    using namespace std::chrono_literals;

    set_thread_name("log");
    hi_log_info("log thread started");

    auto counter_statistics_deadline = std::chrono::utc_clock::now() + 1min;

    while (not stop_token.stop_requested()) {
        log_global.flush();

        hilet now = std::chrono::utc_clock::now();
        if (now >= counter_statistics_deadline) {
            counter_statistics_deadline = now + 1min;
            detail::counter::log();
        }

        std::this_thread::sleep_for(100ms);
    }

    hi_log_info("log thread finished");
}

void log::subsystem_deinit() noexcept
{
    if (global_state_disable(global_state_type::log_is_running)) {
        if (_log_thread.joinable()) {
            _log_thread.request_stop();
            _log_thread.join();
        }

        log_global.flush();
    }
}

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
bool log::subsystem_init() noexcept
{
    _log_thread = std::jthread(log_thread_main);
    return true;
}

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
void log::write(std::string const &str) const noexcept
{
    console_output(str);
}

void log::flush() noexcept
{
    hilet t = trace<"log_flush">{};

    bool wrote_message;
    do {
        std::unique_ptr<detail::log_message_base> copy_of_message;

        {
            hilet lock = std::scoped_lock(_mutex);

            wrote_message = _fifo.take_one([&copy_of_message](auto &message) {
                copy_of_message = message.make_unique_copy();
            });
        }

        if (wrote_message) {
            hi_assert_not_null(copy_of_message);
            write(copy_of_message->format());
        }
    } while (wrote_message);
}

} // namespace hi::inline v1
