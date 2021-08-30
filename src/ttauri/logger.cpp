// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "logger.hpp"
#include "trace.hpp"
#include "required.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "thread.hpp"
#include "url_parser.hpp"
#include "timer.hpp"
#include "unfair_recursive_mutex.hpp"
#include "console.hpp"
#include <format>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>
#include <thread>

namespace tt {
namespace detail {

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
static void logger_write(std::string const &str) noexcept
{
    console_output(str);
}

unfair_mutex logger_mutex;
std::jthread logger_thread;

static void logger_thread_loop(std::stop_token stop_token) noexcept
{
    using namespace std::literals::chrono_literals;

    set_thread_name("logger");
    tt_log_info("logger thread started");

    auto counter_statistics_deadline = std::chrono::utc_clock::now() + 1m;

    while (!stop_token.stop_requested()) {
        logger_flush();

        ttlet now = std::chrono::utc_clock::now();
        if (now >= counter_statistics_deadline) {
            counter_statistics_deadline = now + 1m;
            counter::log();
        }

        std::this_thread::sleep_for(100ms);
    }

    tt_log_info("logger thread finished");
}

void logger_deinit() noexcept
{
    if (to_bool(global_state.fetch_and(~global_state_type::logger_is_running) & global_state_type::logger_is_running)) {
        if (logger_thread.joinable()) {
            logger_thread.request_stop();
            logger_thread.join();
        }

        logger_flush();
    }
}

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
bool logger_init() noexcept
{
    logger_thread = std::jthread(logger_thread_loop);
    return true;
}

}

/** Flush all messages from the log_queue directly from this thread.
 */
void logger_flush() noexcept
{
    ttlet t = trace<"log_flush">{};

    bool wrote_message;
    do {
        std::unique_ptr<detail::log_message_base> copy_of_message;

        {
            ttlet lock = std::scoped_lock(detail::logger_mutex);

            wrote_message = detail::log_fifo.take_one([&copy_of_message](auto &message) {
                copy_of_message = message.make_unique_copy();
            });
        }

        if (wrote_message) {
            tt_axiom(copy_of_message);
            detail::logger_write(copy_of_message->format());
        }
    } while (wrote_message);
}

} // namespace tt
