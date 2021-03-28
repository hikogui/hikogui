// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "counters.hpp"
#include "time_stamp_count.hpp"
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
#include "subsystem.hpp"
#include "log_level.hpp"
#include <date/tz.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <tuple>
#include <mutex>
#include <atomic>
namespace tt {
void trace_record() noexcept;
}

namespace tt {
namespace detail {

class log_message_base {
public:
    log_message_base() noexcept = default;
    virtual ~log_message_base() = default;

    log_message_base(log_message_base const &) = delete;
    log_message_base(log_message_base &&) = delete;
    log_message_base &operator=(log_message_base const &) = delete;
    log_message_base &operator=(log_message_base &&) = delete;

    virtual std::string format() const noexcept = 0;
};

template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Values>
class log_message : public log_message_base {
public:
    static_assert(std::is_same_v<decltype(SourceFile)::value_type, char>, "SourceFile must be a basic_fixed_string<char>");
    static_assert(std::is_same_v<decltype(Fmt)::value_type, char>, "Fmt must be a basic_fixed_string<char>");

    template<typename... Args>
    log_message(Args &&...args) noexcept :
        _time_stamp(time_stamp_count::now(std::memory_order::relaxed)), _what(std::forward<Args>(args)...)
    {
    }

    std::string format() const noexcept override
    {
        ttlet time_point = hires_utc_clock::make(_time_stamp);
        ttlet local_timestring = format_iso8601(time_point);

        if constexpr (static_cast<bool>(Level & log_level::statistics)) {
            return fmt::format("{} {:5} {}\n", local_timestring, to_const_string(Level), _what());
        } else {
            return fmt::format("{} {:5} {} ({}:{})\n", local_timestring, to_const_string(Level), _what(), SourceFile, SourceLine);
        }
    }

private:
    time_stamp_count _time_stamp;
    delayed_format<Fmt, Values...> _what;
};

//template<log_level Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
//log_message(time_stamp_count, Args &&...) -> log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>;

static constexpr size_t MAX_MESSAGE_SIZE = 224;
static constexpr size_t MAX_NR_MESSAGES = 4096;

using log_queue_item_type = polymorphic_optional<log_message_base, MAX_MESSAGE_SIZE>;
using log_queue_type = wfree_message_queue<log_queue_item_type, MAX_NR_MESSAGES>;

/** The global log queue contains messages to be displayed by the logger thread.
 */
inline log_queue_type log_queue;

/** Deinitalize the logger system.
 */
tt_no_inline void logger_deinit() noexcept;

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
tt_no_inline bool logger_init() noexcept;

inline std::atomic<bool> logger_is_running = false;

} // namespace detail

/** Get the OS error message from the last error received on this thread.
 */
[[nodiscard]] std::string get_last_error_message() noexcept;

/** Flush all messages from the log_queue directly from this thread.
 * Flushing includes writing the message to a log file or displaying
 * them on the console.
 */
tt_no_inline void logger_flush() noexcept;

/** Start the logger system.
 * Initialize the logger system if it is not already initialized and while the system is not in shutdown-mode.
 * @return true if the logger system is initialized, false when the system is being shutdown.
 */
inline bool logger_start()
{
    return start_subsystem(detail::logger_is_running, false, detail::logger_init, detail::logger_deinit);
}

/** Stop the logger system.
 * De-initialize the logger system if it is initialized.
 */
inline void logger_stop()
{
    return stop_subsystem(detail::logger_is_running, false, detail::logger_deinit);
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
    if (!static_cast<bool>(log_level_global.load(std::memory_order::relaxed) & Level)) {
        return;
    }

    // Add messages in the queue, block when full.
    // * This reduces amount of instructions needed to be executed during logging.
    // * Simplifies logged_fatal_message logic.
    // * Will make sure everything gets logged.
    // * Blocking is bad in a real time thread, so maybe count the number of times it is blocked.

    // Emplace a message directly on the queue.
    detail::log_queue.write<"logger_blocked">()
        ->emplace<detail::log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>>(std::forward<Args>(args)...);

    if (static_cast<bool>(Level & log_level::fatal) || !detail::logger_is_running.load(std::memory_order::relaxed)) {
        // If the logger did not start we will log in degraded mode and log from the current thread.
        // On fatal error we also want to log from the current thread.
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
