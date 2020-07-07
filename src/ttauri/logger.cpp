// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/logger.hpp"
#include "ttauri/trace.hpp"
#include "ttauri/cpu_utc_clock.hpp"
#include "ttauri/globals.hpp"
#include "ttauri/required.hpp"
#include "ttauri/URL.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/thread.hpp"
#include "ttauri/url_parser.hpp"
#include "ttauri/debugger.hpp"
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

std::ostream &operator<<(std::ostream &lhs, source_code_ptr const &rhs) {
    ttlet source_file = filename_from_path(rhs.source_path);

    lhs << source_file;
    lhs << ":";
    lhs << rhs.source_line;
    return lhs;
}

[[noreturn]] void terminateOnFatalError(std::string &&message) noexcept {
    maintenance_timer.stop();

    if (debugger_is_present()) {
        debugger_log(message);
        debugger_break;

    } else {
        debugger_dialogue("Fatal error",
            "Fatal error: {}.\n\n"
            "This is a serious bug in this application, please email support@pokitec.com with the error message above. "
            "Press OK to quit the application.",
            message
        );

    }
    std::terminate();
}

std::string log_message_base::string() const noexcept
{
    ttlet utc_timestamp = cpu_utc_clock::convert(timestamp);
    ttlet local_timestring = format_iso8601(utc_timestamp);

    return fmt::format("{} {:5} {}", local_timestring, to_const_string(level()), message());
}

void logger_type::writeToConsole(std::string str) noexcept {
    if (debugger_is_present()) {
        debugger_log(str);
    } else {
        std::cerr << str << std::endl;
    }
}

void logger_type::writeToFile(std::string str) noexcept {
}

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
void logger_type::write(std::string const &str) noexcept {
    writeToFile(str);
    writeToConsole(str);
}

void logger_type::display_counters() noexcept {
    ttlet keys = counter_map.keys();
    logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:>18} {:>9} {:>10} {:>10}", "total", "delta", "mean", "peak");
    for (ttlet &tag: keys) {
        ttlet [count, count_since_last_read] = read_counter(tag);
        logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:>18} {:>+9} {:10} {:10} {}",
            count,
            count_since_last_read,
            "", "",
            tag_name(tag)
        );
    }
}

void logger_type::display_trace_statistics() noexcept {
    ttlet keys = trace_statistics_map.keys();
    for (ttlet &tag: keys) {
        auto *stat = trace_statistics_map.get(tag, nullptr);
        tt_assert(stat != nullptr);
        ttlet stat_result = stat->read();

        if (stat_result.last_count <= 0) {
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:18n} {:+9n} {:10} {:10} {}",
                stat_result.count,
                stat_result.last_count,
                "", "",
                tag_name(tag)
            );

        } else {
            // XXX not perfect at all.
            ttlet duration_per_iter = format_engineering(stat_result.last_duration / stat_result.last_count);
            ttlet duration_peak = format_engineering(stat_result.peak_duration);
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:18n} {:+9n} {:>10} {:>10} {}",
                stat_result.count,
                stat_result.last_count,
                duration_per_iter,
                duration_peak,
                tag_name(tag)
            );
        }
    }
}

void logger_type::gather_tick(bool last) noexcept
{
    struct gather_tick_tag {};
    ttlet t = trace<gather_tick_tag>{};

    constexpr auto gather_interval = 30s;

    if (last) {
        LOG_INFO("Counter: displaying counters and statistics at end of program");
        display_counters();
        display_trace_statistics();

    } else if (next_gather_time < hires_utc_clock::now()) {
        LOG_INFO("Counter: displaying counters and statistics over the last {} seconds", gather_interval / 1s);
        display_counters();
        display_trace_statistics();

        ttlet now_rounded_to_interval = hires_utc_clock::now().time_since_epoch() / gather_interval;
        next_gather_time = typename hires_utc_clock::time_point(gather_interval * (now_rounded_to_interval + 1));
    }
}

void logger_type::logger_tick() noexcept {
    struct logger_tick_tag {};
    ttlet t = trace<logger_tick_tag>{};

    while (!message_queue.empty()) {
        auto message = message_queue.read();

        ttlet str = (*message)->string();
        write(str);
    }
}


}
