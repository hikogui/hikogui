// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/trace.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/thread.hpp"
#include "TTauri/Foundation/url_parser.hpp"
#include "TTauri/Foundation/debugger.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace TTauri {

using namespace std::literals::chrono_literals;

std::ostream &operator<<(std::ostream &lhs, source_code_ptr const &rhs) {
    let source_file = filename_from_path(rhs.source_path);

    lhs << source_file;
    lhs << ":";
    lhs << rhs.source_line;
    return lhs;
}

[[noreturn]] void terminateOnFatalError(std::string &&message) noexcept {
    Foundation_globals->stopMaintenanceThread();

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
    let utc_timestamp = cpu_utc_clock::convert(timestamp);
    let local_timestring = format_iso8601(utc_timestamp);

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
    let keys = counter_map.keys();
    for (let &tag: keys) {
        let [count, count_since_last_read] = read_counter(tag);
        logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18} {:+9}", tt5_decode(tag), count, count_since_last_read);
    }
}

void logger_type::display_trace_statistics() noexcept {
    let keys = trace_statistics_map.keys();
    for (let &tag: keys) {
        auto *stat = trace_statistics_map.get(tag, nullptr);
        ttauri_assert(stat != nullptr);
        let stat_result = stat->read();

        if (stat_result.last_count <= 0) {
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18n} {:18n}",
                tt5_decode(tag),
                stat_result.count,
                stat_result.last_count
            );

        } else {
            // XXX not perfect at all.
            let duration_per_iter = format_engineering(stat_result.last_duration / stat_result.last_count);
            let duration_peak = format_engineering(stat_result.peak_duration);
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18n} {:+9n} mean: {}/iter, peak: {}",
                tt5_decode(tag),
                stat_result.count,
                stat_result.last_count, duration_per_iter, duration_peak
            );
        }
    }
}

void logger_type::gather_tick(bool last) noexcept
{
    let t = trace<"gather_tick"_tag>{};

    constexpr auto gather_interval = 30s;

    if (last) {
        LOG_INFO("Counter: displaying counters and statistics at end of program");
        display_counters();
        display_trace_statistics();

    } else if (next_gather_time < hires_utc_clock::now()) {
        LOG_INFO("Counter: displaying counters and statistics over the last {} seconds", gather_interval / 1s);
        display_counters();
        display_trace_statistics();

        let now_rounded_to_interval = hires_utc_clock::now().time_since_epoch() / gather_interval;
        next_gather_time = typename hires_utc_clock::time_point(gather_interval * (now_rounded_to_interval + 1));
    }
}

void logger_type::logger_tick() noexcept {
    let t = trace<"logger_tick"_tag>{};

    while (!message_queue.empty()) {
        auto message = message_queue.read();

        let str = (*message)->string();
        write(str);
    }
}


}
