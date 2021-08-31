// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "time_stamp_count.hpp"
#include "hires_utc_clock.hpp"
#include "polymorphic_optional.hpp"
#include "wfree_fifo.hpp"
#include "atomic.hpp"
#include "meta.hpp"
#include "format.hpp"
#include "source_location.hpp"
#include "architecture.hpp"
#include "delayed_format.hpp"
#include "fixed_string.hpp"
#include "subsystem.hpp"
#include "global_state.hpp"
#include "URL.hpp"
#include <chrono>
#include <format>
#include <string>
#include <string_view>
#include <tuple>
#include <mutex>
#include <atomic>
#include <memory>

namespace tt {
namespace detail {

class log_message_base {
public:
    tt_force_inline log_message_base() noexcept = default;
    virtual ~log_message_base() = default;

    [[nodiscard]] virtual std::string format() const noexcept = 0;
    [[nodiscard]] virtual std::unique_ptr<log_message_base> make_unique_copy() const noexcept = 0;
};

template<global_state_type Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Values>
class log_message : public log_message_base {
public:
    static_assert(std::popcount(to_underlying(Level)) == 1);
    static_assert(std::is_same_v<decltype(SourceFile)::value_type, char>, "SourceFile must be a basic_fixed_string<char>");
    static_assert(std::is_same_v<decltype(Fmt)::value_type, char>, "Fmt must be a basic_fixed_string<char>");

    // clang-format off
    static constexpr char const *log_level_name =
        Level == global_state_type::log_fatal ? "fatal" :
        Level == global_state_type::log_error ? "error" :
        Level == global_state_type::log_warning ? "warning" :
        Level == global_state_type::log_info ? "info" :
        Level == global_state_type::log_debug ? "debug" :
        Level == global_state_type::log_trace ? "trace" :
        Level == global_state_type::log_audit ? "audit" :
        Level == global_state_type::log_statistics ? "stats" :
        "<unknown log level>";
    // clang-format on

    log_message(log_message const &) noexcept = default;
    log_message &operator=(log_message const &) noexcept = default;

    template<typename... Args>
    tt_force_inline log_message(Args &&...args) noexcept :
        _time_stamp(time_stamp_count::inplace_with_thread_id{}), _what(std::forward<Args>(args)...)
    {
    }

    std::string format() const noexcept override
    {
        ttlet time_point = hires_utc_clock::make(_time_stamp);
        ttlet local_timestring = format_iso8601(time_point);
        ttlet cpu_id = _time_stamp.cpu_id();
        ttlet thread_id = _time_stamp.thread_id();

        if constexpr (to_bool(Level & global_state_type::log_statistics)) {
            return std::format("{} {:2}:{:<10} {:5} {}\n", local_timestring, cpu_id, thread_id, log_level_name, _what());
        } else {
            return std::format(
                "{} {:2}:{:<10} {:5} {} ({}:{})\n",
                local_timestring,
                cpu_id,
                thread_id,
                log_level_name,
                _what(),
                URL::urlFromPath(SourceFile).filename(),
                SourceLine);
        }
    }

    [[nodiscard]] std::unique_ptr<log_message_base> make_unique_copy() const noexcept override
    {
        return std::make_unique<log_message>(*this);
    }

private:
    time_stamp_count _time_stamp;
    delayed_format<Fmt, Values...> _what;
};

/** The global log queue contains messages to be displayed by the logger thread.
 */
inline wfree_fifo<log_message_base, 64> log_fifo;

/** Deinitalize the logger system.
 */
tt_no_inline void logger_deinit() noexcept;

/** Initialize the log system.
 * This will start the logging threads which periodically
 * checks the log_queue for new messages and then
 * call log_flush_messages().
 */
tt_no_inline bool logger_init() noexcept;

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
 *
 * Initialize the logger system if it is not already initialized and while the system is not in shutdown-mode.
 *
 * @param log_level The level at which to log.
 * @return true if the logger system is initialized, false when the system is being shutdown.
 */
inline bool logger_start(global_state_type log_level = global_state_type::log_level_default)
{
    set_log_level(log_level);
    return start_subsystem(global_state_type::logger_is_running, detail::logger_init, detail::logger_deinit);
}

/** Stop the logger system.
 * De-initialize the logger system if it is initialized.
 */
inline void logger_stop()
{
    return stop_subsystem(detail::logger_deinit);
}

/** Log a message.
 * @tparam Level log level of message, must be greater or equal to the log level of the `system_status`.
 * @tparam SourceFile The source file where this function was called.
 * @tparam SourceLine The source line where this function was called.
 * @tparam Fmt The format string.
 * @param timestamp The timestamp when the message is logged.
 * @param args Arguments to std::format.
 */
template<global_state_type Level, basic_fixed_string SourceFile, int SourceLine, basic_fixed_string Fmt, typename... Args>
tt_force_inline void log(Args &&...args) noexcept
{
    static_assert(std::popcount(to_underlying(Level)) == 1);

    ttlet state = global_state.load(std::memory_order::relaxed);
    if (not to_bool(state & Level)) {
        return;
    }

    // Add messages in the queue, block when full.
    // * This reduces amount of instructions needed to be executed during logging.
    // * Simplifies logged_fatal_message logic.
    // * Will make sure everything gets logged.
    // * Blocking is bad in a real time thread, so maybe count the number of times it is blocked.

    // Emplace a message directly on the queue.
    detail::log_fifo.emplace<detail::log_message<Level, SourceFile, SourceLine, Fmt, forward_value_t<Args>...>>(
        std::forward<Args>(args)...);

    if (to_bool(Level & global_state_type::log_fatal) or not to_bool(state & global_state_type::logger_is_running)) {
        // If the logger did not start we will log in degraded mode and log from the current thread.
        // On fatal error we also want to log from the current thread.
        [[unlikely]] logger_flush();
    }

    if constexpr (to_bool(Level & global_state_type::log_fatal)) {
        std::terminate();
    }
}

} // namespace tt

#define tt_log_debug(fmt, ...) ::tt::log<::tt::global_state_type::log_debug, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_info(fmt, ...) ::tt::log<::tt::global_state_type::log_info, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_statistics(fmt, ...) ::tt::log<::tt::global_state_type::log_statistics, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_trace(fmt, ...) ::tt::log<::tt::global_state_type::log_trace, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_audit(fmt, ...) ::tt::log<::tt::global_state_type::log_audit, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_warning(fmt, ...) ::tt::log<::tt::global_state_type::log_warning, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_error(fmt, ...) ::tt::log<::tt::global_state_type::log_error, __FILE__, __LINE__, fmt>(__VA_ARGS__)
#define tt_log_fatal(fmt, ...) \
    ::tt::log<::tt::global_state_type::log_fatal, __FILE__, __LINE__, fmt>(__VA_ARGS__); \
    tt_unreachable()
