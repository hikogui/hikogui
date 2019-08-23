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

using log_level = int;
//! Messages that are used for debugging during developmet.
static constexpr log_level log_level_Debug = 0;
//! Informational messages used debugging problems in production by users of the application.
static constexpr log_level log_level_Info = 1;
//! An exception was throw, probably isn't a problem.
static constexpr log_level log_level_Exception = 2;
//! Messages for auditing purposes.
static constexpr log_level log_level_Audit = 3;
//! An error was detected which is recoverable by the application.
static constexpr log_level log_level_Warning = 4;
//! An error was detected and is recoverable by the user.
static constexpr log_level log_level_Error = 5;
//! An error has caused data to be corrupted.
static constexpr log_level log_level_Critical = 6;
//! Unrecoverable error, need to terminate the application to reduce impact.
static constexpr log_level log_level_Fatal = 7;

constexpr char const *log_level_to_const_string(log_level level) noexcept
{
    switch (level) {
    case log_level_Debug:     return "DEBUG";
    case log_level_Info:      return "INFO";
    case log_level_Exception: return "THROW";
    case log_level_Audit:     return "AUDIT";
    case log_level_Warning:   return "WARN";
    case log_level_Error:     return "ERROR";
    case log_level_Critical:  return "CRIT";
    case log_level_Fatal:     return "FATAL";
    default: no_default;
    }
}

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

struct log_message_base {
    virtual std::string string() const noexcept = 0;
    virtual log_level level() const noexcept = 0;
};

template<log_level Level, char const *SourceFile, int SourceLine, char const *Format, typename... Args>
struct log_message: public log_message_base {
    static constexpr char const *LogLevelName = log_level_to_const_string(Level);

    std::tuple<std::decay_t<Args>...> format_args;

    //log_message(log_level level, char const *file, int line, std::decay_t<Args>... format_args) noexcept :
    //    log_message_base(level, file, line), format_args(std::move(format_args)...)
    log_message(Args &&... args) noexcept :
        format_args(std::forward<Args>(args)...)
    {
    }

    log_level level() const noexcept override {
        return Level;
    }

    std::string string() const noexcept override {
        let msg = std::apply([](auto const&... args) {
              return fmt::format(Format, args...);
            },
            format_args
        );

        let source_filename = filename_from_path(SourceFile);

        return fmt::format("{0:14};{1:4} {2:5} {3}", source_filename, SourceLine, LogLevelName, msg);
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
    using message_queue_type = wfree_mpsc_message_queue<message_type,MAX_NR_MESSAGES,MESSAGE_ALIGNMENT>;


    std::atomic<bool> logged_fatal_message = false;

    log_level level = log_level_Debug;

    //! the message queue must work correctly before main() is executed.
    message_queue_type message_queue;

    bool logger_thread_stop = false;
    std::thread logger_thread;

public:
    logger_type(bool test=false) {
        if (!test) {
            logger_thread = std::thread([&]() {
                this->loop();
            });
        }
    }

    ~logger_type() {
        if (logger_thread.joinable()) {
            logger_thread_stop = true;
            logger_thread.join();
        }
    }

    void loop() noexcept;

    template<log_level Level, char const *SourceFile, int SourceLine, char const *Format, typename... Args>
    void log(Args &&... args) noexcept {
        if (Level < logger.level) {
            return;
        }

        if (!message_queue.full()) {
            auto message = message_queue.write();
            // derefence the message so that we get the polymorphic_value, so this assignment will work correctly.
            message->emplace<log_message<Level, SourceFile, SourceLine, Format, Args...>>(std::forward<Args>(args)...);

        } else {
            increment_counter<"log_overflow"_tag>();
        }

        if constexpr (Level >= log_level_Fatal) {
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

// The constructor of logger only starts the logging thread.
// The ring buffer of the logger is trivaliy constructed and can be used before the logger's constructor is stared.
inline logger_type logger = {};

}

constexpr char const *foo(char const *str)
{
    return static_cast<char const *>(str);
}

#define LOGGER_LOG(level, fmt, ...)\
    do {\
        static char const SourceFile[] = __FILE__;\
        static char const FormatStr[] = fmt;\
        logger.log<level, SourceFile, __LINE__, FormatStr>(__VA_ARGS__);\
    } while(false)

#define LOG_DEBUG(fmt, ...) LOGGER_LOG(log_level_Debug, fmt, __VA_ARGS__)
#define LOG_INFO(fmt, ...) LOGGER_LOG(log_level_Info, fmt, __VA_ARGS__)
#define LOG_AUDIT(fmt, ...) LOGGER_LOG(log_level_Audit, fmt, __VA_ARGS__)
#define LOG_WARNING(fmt, ...) LOGGER_LOG(log_level_Warning, fmt, __VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOGGER_LOG(log_level_Error, fmt, __VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) LOGGER_LOG(log_level_Critical, fmt, __VA_ARGS__)
#define LOG_FATAL(fmt, ...) LOGGER_LOG(log_level_Fatal, fmt, __VA_ARGS__)

