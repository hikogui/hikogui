// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "polymorphic_value.hpp"
#include "wfree_message_queue.hpp"
#include "wfree_unordered_map.hpp"
#include "singleton.hpp"
#include "url_parser.hpp"
#include "atomic.hpp"
#include "counters.hpp"
#include "cpu_counter_clock.hpp"
#include "meta.hpp"
#include "format.hpp"
#include <date/tz.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <tuple>

namespace TTauri {

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

// Forward without including trace.hpp
void trace_record() noexcept;

struct source_code_ptr {
    const char *source_path;
    int source_line;

    constexpr source_code_ptr(const char *source_path, int source_line) :
        source_path(source_path), source_line(source_line) {}
};

std::ostream &operator<<(std::ostream &lhs, source_code_ptr const &rhs);

enum class log_level: uint8_t {
    //! Messages that are used for debugging during developmet.
    Debug,
    //! Informational messages used debugging problems in production by users of the application.
    Info,
    //! Trace message.
    Trace,
    //! A counter.
    Counter,
    //! An exception was throw, probably isn't a problem.
    Exception,
    //! Messages for auditing purposes.
    Audit,
    //! An error was detected which is recoverable by the application.
    Warning,
    //! An error was detected and is recoverable by the user.
    Error,
    //! An error has caused data to be corrupted.
    Critical,
    //! Unrecoverable error, need to terminate the application to reduce impact.
    Fatal,
};

constexpr char const *to_const_string(log_level level) noexcept
{
    switch (level) {
    case log_level::Debug:     return "DEBUG";
    case log_level::Info:      return "INFO";
    case log_level::Trace:     return "TRACE";
    case log_level::Counter:   return "COUNT";
    case log_level::Exception: return "THROW";
    case log_level::Audit:     return "AUDIT";
    case log_level::Warning:   return "WARN";
    case log_level::Error:     return "ERROR";
    case log_level::Critical:  return "CRIT";
    case log_level::Fatal:     return "FATAL";
    default: no_default;
    }
}

constexpr bool operator<(log_level lhs, log_level rhs) noexcept { return static_cast<int>(lhs) < static_cast<int>(rhs); }
constexpr bool operator>(log_level lhs, log_level rhs) noexcept { return rhs < lhs; }
constexpr bool operator==(log_level lhs, log_level rhs) noexcept { return static_cast<int>(lhs) == static_cast<int>(rhs); }
constexpr bool operator!=(log_level lhs, log_level rhs) noexcept { return !(lhs == rhs); }
constexpr bool operator<=(log_level lhs, log_level rhs) noexcept { return !(lhs > rhs); }
constexpr bool operator>=(log_level lhs, log_level rhs) noexcept { return !(lhs < rhs); }

struct log_message_base {
    cpu_counter_clock::time_point timestamp;
    char const *format;

    log_message_base(cpu_counter_clock::time_point timestamp, char const *format) noexcept :
        timestamp(timestamp), format(format) {}

    std::string string() const noexcept;
    virtual std::string message() const noexcept = 0;
    virtual log_level level() const noexcept = 0;
};

template<log_level Level, typename... Args>
struct log_message: public log_message_base {
    std::tuple<std::decay_t<Args>...> format_args;

    log_message(cpu_counter_clock::time_point const timestamp, char const *const format, Args &&... args) noexcept :
        log_message_base(timestamp, format), format_args(std::forward<Args>(args)...) {}

    log_level level() const noexcept override {
        return Level;
    }

    std::string message() const noexcept override {
        std::string format_str = format;

        if constexpr (count_type_if<source_code_ptr, Args...>() > 0) {
            if (format_uses_arg_ids(format)) {
                constexpr size_t source_index = index_of_type<source_code_ptr, Args...>();

                format_str += " ({" + std::to_string(source_index) + "})";
            } else {
                format_str += " ({})";
            }
        }

        auto f = [format_str=format_str](auto const&... args) {
                return fmt::format(format_str, args...);
        };

        try {
            return std::apply(f, format_args);
        } catch (fmt::format_error &e) {
            return std::string("ERROR: Could not format '") + format_str + std::string("': ") + e.what();
        }
    }
};


/*! A class with which to log messages to a file or console.
 * This will primarilly used with get_singleton<>().
 */
class logger_type {
    static constexpr size_t MAX_MESSAGE_SIZE = 224;
    static constexpr size_t MESSAGE_ALIGNMENT = 256;
    static constexpr size_t MAX_NR_MESSAGES = 4096;

    using message_type = polymorphic_value<log_message_base,MAX_MESSAGE_SIZE>;
    using message_queue_type = wfree_message_queue<message_type,MAX_NR_MESSAGES>;

    //! the message queue must work correctly before main() is executed.
    message_queue_type message_queue;

    bool logger_thread_stop = false;
    std::thread logger_thread;
    bool gather_thread_stop = false;
    std::thread gather_thread;

public:
    log_level minimum_log_level = log_level::Debug;

    /*! Start logging to file and console.
     */
    void startLogging() noexcept;

    /*! Stop logging to file and console.
     */
    void stopLogging() noexcept;

    /*! Start logging of counters.
     */
    void startStatisticsLogging() noexcept;

    /*! Stop logging of counters.
     */
    void stopStatisticsLogging() noexcept;

    void logger_loop() noexcept;
    void gather_loop() noexcept;

    template<log_level Level, typename... Args>
    force_inline void log(typename cpu_counter_clock::time_point timestamp, char const *format, Args &&... args) noexcept {
        if (Level >= minimum_log_level) {
            // Add messages in the queue, block when full.
            // * This reduces amount of instructions needed to be executed during logging.
            // * Simplifies logged_fatal_message logic.
            // * Will make sure everything gets logged.
            // * Blocking is bad in a real time thread, so maybe count the number of times it is blocked.
            auto message = message_queue.write<"logger_block"_tag>();
            // derefence the message so that we get the polymorphic_value, so this assignment will work correctly.
            message->emplace<log_message<Level, Args...>>(timestamp, format, std::forward<Args>(args)...);

        } else {
            return;
        }

        if constexpr (Level >= log_level::Error) {
            trace_record();
        }

        if constexpr (Level >= log_level::Fatal) {
            // Make sure everything including this message and counters are logged.
            stopStatisticsLogging();
            stopLogging();
            std::terminate();
        }
    }

private:
    void write(std::string const &str) noexcept;
    void writeToFile(std::string str) noexcept;
    void writeToConsole(std::string str) noexcept;
    void display_counters() noexcept;
    void display_trace_statistics() noexcept;
};

// The constructor of logger only starts the logging thread.
// The ring buffer of the logger is trivaliy constructed and can be used before the logger's constructor is stared.
inline logger_type logger = {}; 

}

#define TTAURI_LOG(level, ...) ::TTauri::logger.log<level>(cpu_counter_clock::now(), __VA_ARGS__, source_code_ptr(__FILE__, __LINE__))

#define LOG_DEBUG(...) TTAURI_LOG(log_level::Debug, __VA_ARGS__)
#define LOG_INFO(...) TTAURI_LOG(log_level::Info, __VA_ARGS__)
#define LOG_AUDIT(...) TTAURI_LOG(log_level::Audit, __VA_ARGS__)
#define LOG_EXCEPTION(...) TTAURI_LOG(log_level::Exception, __VA_ARGS__)
#define LOG_WARNING(...) TTAURI_LOG(log_level::Warning, __VA_ARGS__)
#define LOG_ERROR(...) TTAURI_LOG(log_level::Error, __VA_ARGS__)
#define LOG_CRITICAL(...) TTAURI_LOG(log_level::Critical, __VA_ARGS__)
#define LOG_FATAL(...) TTAURI_LOG(log_level::Fatal, __VA_ARGS__)


