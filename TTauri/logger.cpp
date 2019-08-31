// Copyright 2019 Pokitec
// All rights reserved.

#include "logger.hpp"
#include "strings.hpp"
#include "os_detect.hpp"
#include "URL.hpp"
#include "hiperf_utc_clock.hpp"
#include "Application.hpp"
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


std::string log_message_base::string() const noexcept
{
    let source_filename = filename_from_path(source_path);

    let utc_timestamp = hiperf_utc_clock::convert(timestamp);
    let local_timestring = format_full_datetime(utc_timestamp, application().time_zone);

    if (level() == log_level::Counter) {
        return fmt::format("{} {:5} {}", local_timestring, to_const_string(level()), message());
    } else {
        return fmt::format("{} {:5} {}.    {}:{}", local_timestring, to_const_string(level()), message(), source_filename, source_line);
    }
}

/*! Start logging to file and console.
*/
void logger_type::startLogging() noexcept
{
    logger_thread = std::thread([&]() {
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
        this->gather_loop();
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
        } while (hires_utc_clock::now() < next_dump_time && !last_iteration);

        let keys = counter_map.keys();
        LOG_INFO("Counter: displaying {} counters over the last {} seconds.", keys.size(), gather_interval / 1s);

        for (let &tag: keys) {
            let [count, count_since_last_read] = read_counter(tag);
            LOG_COUNTER("{:13} {:18} {:+9}", tag_to_string(tag), count, count_since_last_read);
        }
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
