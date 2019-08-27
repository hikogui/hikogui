// Copyright 2019 Pokitec
// All rights reserved.

#include "logger.hpp"
#include "strings.hpp"
#include "os_detect.hpp"
#include "URL.hpp"
#include "hires_utc_clock.hpp"
#include "sync_clock.hpp"
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
    let local_timestring = format_full_datetime(utc_timestamp, current_time_zone);

    if (level() == log_level::Counter) {
        return fmt::format("{} {:5} {}", local_timestring, to_const_string(level()), message());
    } else {
        return fmt::format("{} {:5} {}.    {}:{}", local_timestring, to_const_string(level()), message(), source_filename, source_line);
    }
}

logger_type::logger_type(bool test) noexcept {
    // The logger is the first object that will use the timezone database.
    // Zo we will initialize it here.
#if USE_OS_TZDB == 0
    let tzdata_location = URL::urlFromResourceDirectory() / "tzdata";
    date::set_install(tzdata_location.nativePath());
#endif
    current_time_zone = date::current_zone();

    // Compiler bug: inline global variables should be constructed only once.
    required_assert(increment_counter<"logger_ctor"_tag>() == 1);

    if (!test) {
        logger_thread = std::thread([&]() {
            this->logger_loop();
        });

        gather_thread = std::thread([&]() {
            this->gather_loop();
        });
    }
}

void logger_type::finish_logging() noexcept {
    // Make sure that all counter and statistics are logged.
    if (gather_thread.joinable()) {
        gather_thread_stop = true;
        gather_thread.join();
    }

    // Make sure all messages have been logged to log-file or console.
    if (logger_thread.joinable()) {
        logger_thread_stop = true;
        logger_thread.join();
    }
}

logger_type::~logger_type() {
    finish_logging();
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

        auto found_fatal_message = false;
        while (!message_queue.empty()) {
            auto message = message_queue.read();

            let str = (*message)->string();
            if ((*message)->level() >= log_level::Fatal) {
                found_fatal_message = true;
            }

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
