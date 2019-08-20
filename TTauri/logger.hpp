// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "polymorphic_value.hpp"
#include "wfree_mpsc_message_queue.hpp"
#include <fmt/format.h>
#include <boost/log/trivial.hpp>
#include <string>
#include <tuple>

namespace TTauri {

enum class log_level {
    //! Messages that are used for debugging during developmet.
    Debug,
    //! Informational messages used debugging problems in production by users of the application.
    Info,
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

constexpr bool operator<(log_level lhs, log_level rhs) { return static_cast<int>(lhs) < static_cast<int>(rhs); }
constexpr bool operator>(log_level lhs, log_level rhs) { return rhs < lhs; }
constexpr bool operator<=(log_level lhs, log_level rhs) { return !(lhs > rhs); }
constexpr bool operator>=(log_level lhs, log_level rhs) { return !(lhs < rhs); }

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

/*! Count number of path seperators in constant string,
 */
constexpr char const *make_relative_path(char const *base_path, char const *absolute_path)
{
    auto relative_path = absolute_path;

    while (*base_path != '\0' && *absolute_path != '\0' && *base_path != *absolute_path) {
        if (*base_path == '/' || *base_path == '\\') {
            relative_path = absolute_path + 1;
        }

        base_path++;
        absolute_path++;
    }


    return relative_path;
}

constexpr char const *make_relative_source_path(char const *source_path) {
    return make_relative_path(__FILE__, source_path);
}

#define TTAURI_SOURCE_FILE make_relative_source_path(__FILE__)


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
    std::tuple<Args...> format_args;

    log_message(log_level level, char const *file, int line, Args const&... formats_args) noexcept :
        log_message_base(level, file, line), format_args(formats_args...) {}

    std::string string() const noexcept override {
        let msg = std::apply([](auto const&... args) {
              return fmt::format(args...);
            },
            format_args
        );
        return msg;
    }
};


/*! A class with which to log messages to a file or console.
 * This will primarilly used with get_singleton<>().
 */
class logger {
    static constexpr size_t MAX_MESSAGE_SIZE = 256;
    static constexpr size_t MAX_NR_MESSAGES = 4096;

    using message_type = polymorphic_value<log_message_base,MAX_MESSAGE_SIZE>;
    using message_queue_type = wfree_mpsc_message_queue<message_type,MAX_NR_MESSAGES>;

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
        if (loggingThread.joinable()) {
            logger_thread_stop = true;
            logger_thread.join();
        }
    }

    void loop() noexcept;

    template<typename... Args>
    void log(log_level level, char const *source_file, int source_line, Args const &... format_args) noexcept {
        if (level < logger::level) {
            return;
        }

        // XXX add timestamp.
        let message = log_message{level, source_file, source_line, format_args...};
        message_queue.push(message);

        if (level == log_level::Fatal) {
            // XXX Make sureeverything gets logged
            std::terminate();
        }
    }
};

}

#define LOG_DEBUG(...) get_singleton<logger>().log(log_level::Debug, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_INFO(...) get_singleton<logger>().log(log_level::Info, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_AUDIT(...) get_singleton<logger>().log(log_level::Audit, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_WARNING(...) get_singleton<logger>().log(log_level::Warning, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_ERROR(...) get_singleton<logger>().log(log_level::Error, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_CRITICAL(...) get_singleton<logger>().log(log_level::Critical, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)
#define LOG_FATAL(...) get_singleton<logger>().log(log_level::Fatal, TTAURI_SOURCE_FILE, __LINE__, __VA_ARGS__)

