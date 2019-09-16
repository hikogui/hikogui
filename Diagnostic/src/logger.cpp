// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Diagnostic/trace.hpp"
#include "TTauri/Time/hiperf_utc_clock.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/required.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/strings.hpp"
#include "TTauri/Required/thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <chrono>

#if OPERATING_SYSTEM == OS_WINDOWS
#include <Windows.h>
#include <debugapi.h>
#endif

namespace TTauri {

using namespace std::literals::chrono_literals;

std::ostream &operator<<(std::ostream &lhs, source_code_ptr const &rhs) {
    let source_file = filename_from_path(rhs.source_path);

    lhs << source_file;
    lhs << ":";
    lhs << rhs.source_line;
    return lhs;
}

std::string log_message_base::string() const noexcept
{
    let utc_timestamp = hiperf_utc_clock::convert(timestamp);
    let local_timestring = format_iso8601(utc_timestamp);

    return fmt::format("{} {:5} {}", local_timestring, to_const_string(level()), message());
}

/*! Start logging to file and console.
*/
void logger_type::startLogging() noexcept
{
    logger_thread = std::thread([&]() {
        set_thread_name("LoggingThread");
        this->logger_loop();
    });
}

/*! Stop logging to file and console.
*/
void logger_type::stopLogging() noexcept
{
    // Make sure all messages have been logged to log-file or console.
    if (logger_thread.joinable()) {
        logger_thread_stop = true;
        logger_thread.join();
    }
}

/*! Start logging of counters.
*/
void logger_type::startStatisticsLogging() noexcept
{
    gather_thread = std::thread([&]() {
        set_thread_name("Statistics");
        LOG_AUDIT("Started: statistics gathering thread.");
        this->gather_loop();
        LOG_AUDIT("Finished: statsistics gathering thread.");
    });
}

/*! Stop logging of counters.
*/
void logger_type::stopStatisticsLogging() noexcept
{
    // Make sure that all counter and statistics are logged.
    if (gather_thread.joinable()) {
        gather_thread_stop = true;
        gather_thread.join();
    }
}

void logger_type::writeToFile(std::string str) noexcept {
}

void logger_type::writeToConsole(std::string str) noexcept {
#if OPERATING_SYSTEM == OS_WINDOWS
    str += "\r\n";
    OutputDebugStringW(translateString<std::wstring>(str).data());
#else
    cerr << str << endl;
#endif
}

/*! Write to a log file and console.
 * This will write to the console if one is open.
 * It will also create a log file in the application-data directory.
 */
void logger_type::write(std::string const &str) noexcept {
    writeToFile(str);
    writeToConsole(str);
}

void logger_type::display_time_calibration() noexcept {
    if (let message = Time_globals->read_message()) {
        LOG_AUDIT("{}", *message);
    }
}

void logger_type::display_counters() noexcept {
    let keys = counter_map.keys();
    for (let &tag: keys) {
        let [count, count_since_last_read] = read_counter(tag);
        logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18} {:+9}", tag_to_string(tag), count, count_since_last_read);
    }
}

void logger_type::display_trace_statistics() noexcept {
    let keys = trace_statistics_map.keys();
    for (let &tag: keys) {
        auto *stat = trace_statistics_map.get(tag, nullptr);
        required_assert(stat != nullptr);
        let stat_result = stat->read();

        if (stat_result.last_count <= 0) {
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18n} {:18n}",
                tag_to_string(tag),
                stat_result.count,
                stat_result.last_count
            );

        } else {
            // XXX not perfect at all.
            logger.log<log_level::Counter>(cpu_counter_clock::now(), "{:13} {:18n} {:+9n} mean: {:n} ns/iter, peak: {:n} ns/iter",
                tag_to_string(tag),
                stat_result.count,
                stat_result.last_count, (stat_result.last_duration / stat_result.last_count) / 1ns, stat_result.peak_duration / 1ns
            );
        }
    }
}

void logger_type::gather_loop() noexcept {
    constexpr auto gather_interval = 30s;
    bool last_iteration = false;

    do {
        let now_rounded_to_interval = hires_utc_clock::now().time_since_epoch() / gather_interval;
        let next_dump_time = typename hires_utc_clock::time_point(gather_interval * (now_rounded_to_interval + 1));

        do {
            std::this_thread::sleep_for(100ms);

            if (gather_thread_stop) {
                // We need to log all counter before finishing.
                last_iteration = true;
            }

            display_time_calibration();
        } while (hires_utc_clock::now() < next_dump_time && !last_iteration);

        if (last_iteration) {
            LOG_INFO("Counter: displaying counters and statistics at end of program");
        } else {
            LOG_INFO("Counter: displaying counters and statistics over the last {} seconds", gather_interval / 1s);
        }

        display_counters();
        display_trace_statistics();

    } while (!last_iteration);
}


void logger_type::logger_loop() noexcept {
    bool last_iteration = false;

    do {
        std::this_thread::sleep_for(100ms);

        if (logger_thread_stop) {
            // We need to log everything to the logfile and console before finishing.
            last_iteration = true;
        }

        while (!message_queue.empty()) {
            auto message = message_queue.read();

            let str = (*message)->string();
            write(str);
        }
    } while (!last_iteration);
}

gsl_suppress(i.11)
std::string getLastErrorMessage()
{
    DWORD const errorCode = GetLastError();
    size_t const messageSize = 32768;
    wchar_t* const c16_message = new wchar_t[messageSize];;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, // source
        errorCode,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        c16_message,
        messageSize,
        NULL
    );

    let message = translateString<std::string>(std::wstring(c16_message));
    delete [] c16_message;

    return message;
}

}
