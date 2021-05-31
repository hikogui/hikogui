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

using namespace std::literals::chrono_literals;

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
static void logger_write(std::string const &str) noexcept
{
    console_output(str);
}

unfair_recursive_mutex logger_mutex;
std::jthread logger_thread;



static void logger_thread_loop(std::stop_token stop_token) noexcept
{
    set_thread_name("logger");
    tt_log_info("logger thread started");

    while (!stop_token.stop_requested()) {
        logger_flush();
        std::this_thread::sleep_for(100ms);
    }

    tt_log_info("logger thread finished");
}

void logger_deinit() noexcept
{
    ttlet lock = std::scoped_lock(logger_mutex);

    if (logger_thread.joinable()) {
        logger_thread.request_stop();
        logger_thread.join();
    }

    logger_flush();
}

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
bool logger_init() noexcept
{
    ttlet lock = std::scoped_lock(logger_mutex);
    logger_thread = std::jthread(logger_thread_loop);
    return true;
}

}

/** Flush all messages from the log_queue directly from this thread.
 */
void logger_flush() noexcept
{
    ttlet t = trace<"log_flush">{};
    ttlet lock = std::scoped_lock(detail::logger_mutex);

    while (!detail::log_queue.empty()) {
        auto message = detail::log_queue.read();

        detail::logger_write((*message)->format());

        // Call the virtual-destructor of the `log_message_base`, so that it can skip this when
        // adding messages to the queue.
        message->reset();
    }
}

} // namespace tt
