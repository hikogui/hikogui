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
#include "system_status.hpp"
#include <date/tz.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <tuple>
#include <mutex>

namespace tt {

enum class log_level : uint8_t {
    debug = to_log_level(system_status_type::log_level_debug),
    info = to_log_level(system_status_type::log_level_info),
    statistics = to_log_level(system_status_type::log_level_statistics),
    trace = to_log_level(system_status_type::log_level_trace),
    audit = to_log_level(system_status_type::log_level_audit),
    warning = to_log_level(system_status_type::log_level_warning),
    error = to_log_level(system_status_type::log_level_error),
    fatal = to_log_level(system_status_type::log_level_fatal)
};

[[nodiscard]] constexpr log_level operator&(log_level const &lhs, log_level const &rhs) noexcept
{
    return static_cast<log_level>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}

[[nodiscard]] constexpr log_level operator|(log_level const &lhs, log_level const &rhs) noexcept
{
    return static_cast<log_level>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}

constexpr log_level &operator|=(log_level &lhs, log_level const &rhs) noexcept
{
    lhs = lhs | rhs;
    return lhs;
}

/** Make a log level mask based on a user given log level.
 */
[[nodiscard]] constexpr log_level make_log_level(log_level user_level) noexcept
{
    auto r = log_level{};

    switch (user_level) {
        using enum log_level;
    case debug: r |= log_level::debug; [[fallthrough]];
    case info: r |= info; [[fallthrough]];
    case warning:
        r |= statistics;
        r |= warning;
        [[fallthrough]];
    case error:
        r |= trace;
        r |= error;
        r |= fatal;
        r |= audit;
        break;
    default: tt_no_default();
    }

    return r;
}

constexpr char const *to_const_string(log_level level) noexcept
{
    if (level >= log_level::fatal) {
        return "fatal";
    } else if (level >= log_level::error) {
        return "error";
    } else if (level >= log_level::warning) {
        return "warning";
    } else if (level >= log_level::audit) {
        return "audit";
    } else if (level >= log_level::trace) {
        return "trace";
    } else if (level >= log_level::statistics) {
        return "statistics";
    } else if (level >= log_level::info) {
        return "info";
    } else if (level >= log_level::debug) {
        return "debug";
    } else {
        return "none";
    }
}

inline void system_status_set_log_level(log_level level) noexcept
{
    return system_status_set_log_level(static_cast<uint8_t>(level));
}

inline int command_line_argument_to_log_level(std::string_view str)
{
    if (str == "debug") {
        return static_cast<int>(make_log_level(log_level::debug));

    } else if (str == "info") {
        return static_cast<int>(make_log_level(log_level::info));

    } else if (str == "warning") {
        return static_cast<int>(make_log_level(log_level::warning));

    } else if (str == "error") {
        return static_cast<int>(make_log_level(log_level::error));

    } else {
        throw parse_error("Unknown log level '{}'", str);
    }
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
    log_message(Args &&...args) noexcept :
        _timestamp(::tt::cpu_counter_clock::now()), _what(std::forward<Args>(args)...)
    {
    }

    std::string format() const noexcept override
    {
        ttlet local_timestring = log_message_base::cpu_utc_clock_as_iso8601(_timestamp);

        if constexpr (static_cast<bool>(Level & log_level::statistics)) {
            return fmt::format("{} {:5} {}", local_timestring, to_const_string(Level), _what());
        } else {
            return fmt::format("{} {:5} {} ({}:{})", local_timestring, to_const_string(Level), _what(), SourceFile, SourceLine);
        }
    }

private:
    cpu_counter_clock::time_point _timestamp;
    delayed_format<Fmt, Values...> _what;
};

template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
log_message(cpu_counter_clock::time_point, Args &&...)
    -> log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>;

static constexpr size_t MAX_MESSAGE_SIZE = 224;
static constexpr size_t MAX_NR_MESSAGES = 4096;

using log_queue_item_type = polymorphic_optional<log_message_base, MAX_MESSAGE_SIZE>;
using log_queue_type = wfree_message_queue<log_queue_item_type, MAX_NR_MESSAGES>;

/** The global log queue contains messages to be displayed by the logger thread.
 */
inline log_queue_type log_queue;

std::string getLastErrorMessage();

// Forward without including trace.hpp
void trace_record() noexcept;

/** Flush all messages from the log_queue directly from this thread.
 */
tt_no_inline void logger_flush() noexcept;

/** Deinitalize the logger system.
 */
tt_no_inline void logger_deinit() noexcept;

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
tt_no_inline void logger_init() noexcept;

/** Start the logger system.
 * Initialize the logger system if it is not already initialized and while the system is not in shutdown-mode.
 * @return true if the logger system is initialized, false when the system is being shutdown.
 */
inline bool logger_start()
{
    return system_status_start_subsystem(system_status_type::logger, logger_init, logger_deinit);
}

/** Log a message.
 * @tparam Level log level of message, must be greater or equal to the log level of the `system_status`.
 * @tparam SourceFile The source file where this function was called.
 * @tparam SourceLine The source line where this function was called.
 * @tparam Fmt The format string.
 * @param timestamp The timestamp when the message is logged.
 * @param args Arguments to fmt::format.
 */
template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
void log(Args &&...args) noexcept
{
    ttlet status = system_status.load(std::memory_order::memory_order_relaxed);

    if (!static_cast<bool>(to_log_level(status) & static_cast<uint8_t>(Level))) [[likely]] {
        return;
    }

    // Add messages in the queue, block when full.
    // * This reduces amount of instructions needed to be executed during logging.
    // * Simplifies logged_fatal_message logic.
    // * Will make sure everything gets logged.
    // * Blocking is bad in a real time thread, so maybe count the number of times it is blocked.
    //auto message = ;

    // Emplace a message directly on the queue.
    log_queue.write<"logger_blocked">()->emplace<log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>>(
        std::forward<Args>(args)...);

    if (static_cast<bool>(Level & log_level::fatal) || !logger_start()) {
        [[unlikely]] logger_flush();
    }

    if constexpr (static_cast<bool>(Level & log_level::fatal)) {
        std::terminate();

    } else if constexpr (static_cast<bool>(Level & log_level::error)) {
        // Actually logging of tracing will only work when we cleanly unwind the stack and destruct all trace objects.
        trace_record();
    }
}

} // namespace tt

#define tt_log(level, fmt, ...) \
    do { \
        ::tt::log<level, __FILE__, __LINE__, fmt>(__VA_ARGS__); \
    } while (false)

#define tt_log_debug(fmt, ...) tt_log(::tt::log_level::debug, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_info(fmt, ...) tt_log(::tt::log_level::info, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_statistics(fmt, ...) tt_log(::tt::log_level::statistics, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_trace(fmt, ...) tt_log(::tt::log_level::trace, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_audit(fmt, ...) tt_log(::tt::log_level::audit, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_warning(fmt, ...) tt_log(::tt::log_level::warning, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_error(fmt, ...) tt_log(::tt::log_level::error, fmt __VA_OPT__(, ) __VA_ARGS__)
#define tt_log_fatal(fmt, ...) \
    tt_log(::tt::log_level::fatal, fmt __VA_OPT__(, ) __VA_ARGS__); \
    tt_unreachable()
