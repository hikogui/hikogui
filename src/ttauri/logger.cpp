// Copyright 2019 Pokitec
// All rights reserved.

#include "logger.hpp"
#include "trace.hpp"
#include "cpu_utc_clock.hpp"
#include "required.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "thread.hpp"
#include "url_parser.hpp"
#include "debugger.hpp"
#include "timer.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace tt {

using namespace std::literals::chrono_literals;

[[nodiscard]] std::string log_message_base::cpu_utc_clock_as_iso8601(cpu_counter_clock::time_point const timestamp) noexcept
{
    return format_iso8601(cpu_utc_clock::convert(timestamp));
}

[[noreturn]] void terminateOnFatalError(std::string &&message) noexcept
{
    if (debugger_is_present()) {
        debugger_log(message);
        tt_debugger_break();

    } else {
        debugger_dialogue(
            "Fatal error",
            "Fatal error: {}.\n\n"
            "This is a serious bug in this application, please email support@pokitec.com with the error message above. "
            "Press OK to quit the application.",
            message);
    }
    std::terminate();
}

void logger_type::writeToConsole(std::string str) noexcept
{
    if (debugger_is_present()) {
        debugger_log(str);
    } else {
        std::cerr << str << std::endl;
    }
}

void logger_type::writeToFile(std::string str) noexcept {}

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
void logger_type::write(std::string const &str) noexcept
{
    writeToFile(str);
    writeToConsole(str);
}

void logger_type::display_counters() noexcept
{
    ttlet keys = counter_map.keys();
    tt_log_counter("{:>18} {:>9} {:>10} {:>10}", "total", "delta", "mean", "peak");
    for (ttlet &tag : keys) {
        ttlet[count, count_since_last_read] = read_counter(tag);
        tt_log_counter("{:>18} {:>+9} {:10} {:10} {}", count, count_since_last_read, "", "", tag_name(tag));
    }
}

void logger_type::display_trace_statistics() noexcept
{
    ttlet keys = trace_statistics_map.keys();
    for (ttlet &tag : keys) {
        auto *stat = trace_statistics_map.get(tag, nullptr);
        tt_assert(stat != nullptr);
        ttlet stat_result = stat->read();

        if (stat_result.last_count <= 0) {
            tt_log_counter("{:18n} {:+9n} {:10} {:10} {}", stat_result.count, stat_result.last_count, "", "", tag_name(tag));

        } else {
            // XXX not perfect at all.
            ttlet duration_per_iter = format_engineering(stat_result.last_duration / stat_result.last_count);
            ttlet duration_peak = format_engineering(stat_result.peak_duration);
            tt_log_counter(
                "{:18n} {:+9n} {:>10} {:>10} {}",
                stat_result.count,
                stat_result.last_count,
                duration_per_iter,
                duration_peak,
                tag_name(tag));
        }
    }
}

void logger_type::gather_tick(bool last) noexcept
{
    struct gather_tick_tag {
    };
    ttlet t = trace<gather_tick_tag>{};

    constexpr auto gather_interval = 30s;

    if (last) {
        tt_log_info("Counter: displaying counters and statistics at end of program");
        display_counters();
        display_trace_statistics();

    } else if (next_gather_time < hires_utc_clock::now()) {
        tt_log_info("Counter: displaying counters and statistics over the last {} seconds", gather_interval / 1s);
        display_counters();
        display_trace_statistics();

        ttlet now_rounded_to_interval = hires_utc_clock::now().time_since_epoch() / gather_interval;
        next_gather_time = typename hires_utc_clock::time_point(gather_interval * (now_rounded_to_interval + 1));
    }
}

void logger_type::logger_tick() noexcept
{
    struct logger_tick_tag {
    };
    ttlet t = trace<logger_tick_tag>{};

    while (!message_queue.empty()) {
        auto message = message_queue.read();

        ttlet str = (*message)->format();

        write(str);

        // Call the virtual-destructor of the `log_message_base`, so that it can skip this when
        // adding messages to the queue.
        message->reset();
    }
}

} // namespace tt
