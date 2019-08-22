// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "polymorphic_value.hpp"
#include "wfree_mpsc_message_queue.hpp"
#include "singleton.hpp"
#include "url_parser.hpp"
#include "atomic.hpp"
#include "counters.hpp"
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <string>
#include <string_view>
#include <tuple>

namespace TTauri {

enum class log_level {
    //! Messages that are used for debugging during developmet.
    Debug,
    //! Informational messages used debugging problems in production by users of the application.
    Info,
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
    Fatal
};

constexpr char const *to_const_string(log_level level) noexcept
{
    switch (level) {
    case log_level::Debug:     return "DEBUG";
    case log_level::Info:      return "INFO";
    case log_level::Exception: return "THROW";
    case log_level::Audit:     return "AUDIT";
    case log_level::Warning:   return "WARN";
    case log_level::Error:     return "ERROR";
    case log_level::Critical:  return "CRIT";
    case log_level::Fatal:     return "FATAL";
    default: no_default;
    }
}

inline std::ostream &operator<<(std::ostream &lhs, log_level rhs) noexcept { return lhs << to_const_string(rhs); }
constexpr bool operator<(log_level lhs, log_level rhs) { return static_cast<int>(lhs) < static_cast<int>(rhs); }
constexpr bool operator>(log_level lhs, log_level rhs) { return rhs < lhs; }
constexpr bool operator<=(log_level lhs, log_level rhs) { return !(lhs > rhs); }
constexpr bool operator>=(log_level lhs, log_level rhs) { return !(lhs < rhs); }

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

struct log_message_base {
    log_level level;
    char const *file;
    int line;

    log_message_base(log_level level, char const *file, int line) noexcept :
        level(level), file(file), line(line) {}

    virtual std::string string() const noexcept = 0;
};

template<typename... Args>
struct log_message: public log_message_base {
    std::tuple<std::decay_t<Args>...> format_args;

    //log_message(log_level level, char const *file, int line, std::decay_t<Args>... format_args) noexcept :
    //    log_message_base(level, file, line), format_args(std::move(format_args)...)
    log_message(log_level level, char const *file, int line, Args &&... args) noexcept :
        log_message_base(level, file, line), format_args(std::forward<Args>(args)...)
    {
    }

    std::string string() const noexcept override {
        let msg = std::apply([](auto const&... args) {
              return fmt::format(args...);
            },
            format_args
        );

        let filename = filename_from_path(file);

        return fmt::format("{0:14};{1:4} {2:5} {3}", filename, line, level, msg);
    }
};


/*! A class with which to log messages to a file or console.
 * This will primarilly used with get_singleton<>().
 */
class logger {
    static constexpr size_t MAX_MESSAGE_SIZE = 236;
    static constexpr size_t MESSAGE_ALIGNMENT = 256;
    static constexpr size_t MAX_NR_MESSAGES = 4096;

    using message_type = polymorphic_value<log_message_base,MAX_MESSAGE_SIZE>;
    using message_queue_type = wfree_mpsc_message_queue<message_type,MAX_NR_MESSAGES,MESSAGE_ALIGNMENT>;

    std::atomic<bool> logged_fatal_message = false;

    std::atomic<log_level> level = log_level::Debug;
    message_queue_type message_queue = {};

    bool logger_thread_stop = false;
    std::thread logger_thread;

public:
    logger(bool test=false) {
        if (!test) {
            logger_thread = std::thread([&]() {
                this->loop();
            });
        }
    }

    ~logger() {
        if (logger_thread.joinable()) {
            logger_thread_stop = true;
            logger_thread.join();
        }
    }

    void loop() noexcept;

    template<typename... Args>
    void log(log_level level, char const *source_file, int source_line, Args &&... format_args) noexcept {
        if (level < logger::level) {
            return;
        }

        if (!message_queue.full()) {
            auto message = message_queue.write();
            // derefence the message so that we get the polymorphic_value, so this assignment will work correctly.
            message->emplace<log_message<Args...>>(level, source_file, source_line, std::forward<Args>(format_args)...);

        } else {
            increment_counter<"log_overflow"_tag>();
        }

        if (level >= log_level::Fatal) {
            // Wait until the logger-thread is finished.
            wait_for_transition(logged_fatal_message, true);
            std::terminate();
        }
    }

private:
    void write(std::string const &str) noexcept;
    void writeToFile(std::string str) noexcept;
    void writeToConsole(std::string str) noexcept;
};

}

#define LOG_DEBUG(...) get_singleton<logger>().log(log_level::Debug, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) get_singleton<logger>().log(log_level::Info, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_AUDIT(...) get_singleton<logger>().log(log_level::Audit, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) get_singleton<logger>().log(log_level::Warning, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) get_singleton<logger>().log(log_level::Error, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_CRITICAL(...) get_singleton<logger>().log(log_level::Critical, __FILE__, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) get_singleton<logger>().log(log_level::Fatal, __FILE__, __LINE__, __VA_ARGS__)

