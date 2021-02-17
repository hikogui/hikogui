// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "counters.hpp"
#include "cpu_counter_clock.hpp"
#include "hires_utc_clock.hpp"
#include "polymorphic_optional.hpp"
#include "wfree_message_queue.hpp"
#include "atomic.hpp"
#include "meta.hpp"
#include "format.hpp"
#include "source_location.hpp"
#include "os_detect.hpp"
#include "delayed_format.hpp"
#include "fixed_string.hpp"
#include <date/tz.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <tuple>
#include <mutex>

namespace tt {

std::string getLastErrorMessage();

[[noreturn]] void terminateOnFatalError(std::string &&message) noexcept;

// Forward without including trace.hpp
void trace_record() noexcept;

enum class log_level : uint8_t {
    //! Messages that are used for debugging during developmet.
    Debug,
    //! Informational messages used debugging problems in production by users of the application.
    Info,
    //! Trace message.
    Trace,
    //! A counter.
    Counter,
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
    case log_level::Debug: return "DEBUG";
    case log_level::Info: return "INFO";
    case log_level::Trace: return "TRACE";
    case log_level::Counter: return "COUNT";
    case log_level::Audit: return "AUDIT";
    case log_level::Warning: return "WARN";
    case log_level::Error: return "ERROR";
    case log_level::Critical: return "CRIT";
    case log_level::Fatal: return "FATAL";
    default: return "<unknown>";
    }
}

inline int command_line_argument_to_log_level(std::string_view str) noexcept
{
    if (str == "debug") {
        return static_cast<int>(log_level::Debug);
    } else if (str == "info") {
        return static_cast<int>(log_level::Info);
    } else if (str == "audit") {
        return static_cast<int>(log_level::Audit);
    } else if (str == "warning") {
        return static_cast<int>(log_level::Warning);
    } else if (str == "error") {
        return static_cast<int>(log_level::Error);
    } else if (str == "critical") {
        return static_cast<int>(log_level::Critical);
    } else if (str == "fatal") {
        return static_cast<int>(log_level::Fatal);
    } else {
        return -1;
    }
}

constexpr bool operator<(log_level lhs, log_level rhs) noexcept
{
    return static_cast<int>(lhs) < static_cast<int>(rhs);
}
constexpr bool operator>(log_level lhs, log_level rhs) noexcept
{
    return rhs < lhs;
}
constexpr bool operator==(log_level lhs, log_level rhs) noexcept
{
    return static_cast<int>(lhs) == static_cast<int>(rhs);
}
constexpr bool operator!=(log_level lhs, log_level rhs) noexcept
{
    return !(lhs == rhs);
}
constexpr bool operator<=(log_level lhs, log_level rhs) noexcept
{
    return !(lhs > rhs);
}
constexpr bool operator>=(log_level lhs, log_level rhs) noexcept
{
    return !(lhs < rhs);
}

class log_message_base {
public:
    log_message_base() noexcept = default;
    virtual ~log_message_base() = default;

    log_message_base(log_message_base const &) = delete;
    log_message_base(log_message_base &&) = delete;
    log_message_base &operator=(log_message_base const &) = delete;
    log_message_base &operator=(log_message_base &&) = delete;

    virtual std::string format() const noexcept = 0;

    static std::string cpu_utc_clock_as_iso8601(cpu_counter_clock::time_point const timestamp) noexcept;
};

template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Values>
class log_message : public log_message_base {
public:
    static_assert(std::is_same_v<decltype(SourceFile)::value_type, char>, "SourceFile must be a basic_fixed_string<char>");
    static_assert(std::is_same_v<decltype(Fmt)::value_type, char>, "Fmt must be a basic_fixed_string<char>");

    template<typename... Args>
    log_message(cpu_counter_clock::time_point timestamp, Args &&...args) noexcept :
        _timestamp(timestamp), _what(std::forward<Args>(args)...)
    {
    }

    std::string format() const noexcept override
    {
        ttlet local_timestring = log_message_base::cpu_utc_clock_as_iso8601(_timestamp);

        if constexpr (Level == log_level::Trace || Level == log_level::Counter) {
            return fmt::format("{} {:5} {}", local_timestring, to_const_string(Level), _what());
        } else {
            return fmt::format(
                "{} {:5} {} ({}:{})", local_timestring, to_const_string(Level), _what(), SourceFile, SourceLine);
        }
    }

private:
    cpu_counter_clock::time_point _timestamp;
    delayed_format<Fmt, Values...> _what;
};

template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
log_message(cpu_counter_clock::time_point, Args &&...)
    -> log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>;

/*! A class with which to log messages to a file or console.
 */
class logger_type {
    static constexpr size_t MAX_MESSAGE_SIZE = 224;
    static constexpr size_t MESSAGE_ALIGNMENT = 256;
    static constexpr size_t MAX_NR_MESSAGES = 4096;

    using message_type = polymorphic_optional<log_message_base, MAX_MESSAGE_SIZE>;
    using message_queue_type = wfree_message_queue<message_type, MAX_NR_MESSAGES>;

    //! the message queue must work correctly before main() is executed.
    message_queue_type message_queue;

    hires_utc_clock::time_point next_gather_time = {};

public:
    logger_type() noexcept;

    log_level minimum_log_level = log_level::Debug;

    void logger_tick() noexcept;
    void gather_tick(bool last) noexcept;

    template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
    void
    log(typename cpu_counter_clock::time_point timestamp, Args &&...args) noexcept
    {
        if (Level >= minimum_log_level) {
            // Add messages in the queue, block when full.
            // * This reduces amount of instructions needed to be executed during logging.
            // * Simplifies logged_fatal_message logic.
            // * Will make sure everything gets logged.
            // * Blocking is bad in a real time thread, so maybe count the number of times it is blocked.
            auto message = message_queue.write<"logger_blocked">();

            // dereference the message so that we get the polymorphic_optional, so this assignment will work correctly.
            message->emplace<log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>>(
                timestamp, std::forward<Args>(args)...);

            if constexpr (Level >= log_level::Fatal) {
                // Make sure everything including this message and counters are logged.
                terminateOnFatalError((*message)->format());

            } else if constexpr (Level >= log_level::Error) {
                // Actually logging of tracing will only work when we cleanly unwind the stack and destruct all trace objects
                // this will not work on fatal messages.
                trace_record();
            }
        }
    }

private:
    void write(std::string const &str) noexcept;
    void writeToFile(std::string str) noexcept;
    void writeToConsole(std::string str) noexcept;
    void display_time_calibration() noexcept;
    void display_counters() noexcept;
    void display_trace_statistics() noexcept;
};

// The constructor of logger only starts the logging thread.
// The ring buffer of the logger is triviality constructed and can be used before the logger's constructor is stared.
inline logger_type logger = {};

} // namespace tt

#define tt_log(level, fmt, ...) \
    do { \
        ttlet _tt_log_timestamp = ::tt::cpu_counter_clock::now(); \
        ::tt::logger.log<level, __FILE__, __LINE__, fmt>(_tt_log_timestamp __VA_OPT__(,) __VA_ARGS__); \
    } while (false)

#define tt_log_debug(fmt, ...) tt_log(::tt::log_level::Debug, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_trace(fmt, ...) tt_log(::tt::log_level::Trace, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_counter(fmt, ...) tt_log(::tt::log_level::Counter, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_info(fmt, ...) tt_log(::tt::log_level::Info, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_audit(fmt, ...) tt_log(::tt::log_level::Audit, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_warning(fmt, ...) tt_log(::tt::log_level::Warning, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_error(fmt, ...) tt_log(::tt::log_level::Error, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_critical(fmt, ...) tt_log(::tt::log_level::Critical, fmt __VA_OPT__(,) __VA_ARGS__)
#define tt_log_fatal(fmt, ...) \
    tt_log(::tt::log_level::Fatal, fmt __VA_OPT__(,) __VA_ARGS__); \
    tt_unreachable()
