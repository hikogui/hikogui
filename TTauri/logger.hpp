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

#ifdef _WIN32
std::string getLastErrorMessage();
#endif

void initializeLogging() noexcept;

enum class log_level_t {
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

struct log_message_base {
    log_level_t level;
    char const *file;
    int line;

    log_message_base(log_level_t level, char const *file, int line) noexcept :
        level(level), file(file), line(line) {}

    virtual std::string string() const noexcept = 0;
};

template<typename... Args>
struct log_message: public log_message_base {
    std::tuple<Args...> format_args;

    log_message(log_level_t level, char const *file, int line, Args const&... formats_args) noexcept :
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


class logger {
    static constexpr size_t MAX_MESSAGE_SIZE = 128;
    static constexpr size_t MAX_NR_MESSAGES = 256;

    using message_type = polymorphic_value<log_message_base,MAX_MESSAGE_SIZE>;
    using message_queue_type = wfree_mpsc_message_queue<message_type,MAX_NR_MESSAGES>;

    static message_queue_type &message_queue() {
        static message_queue_type intrinsic = {};
        return intrinsic;
    }

public:

    template<typename... Args>
    static void log(log_level_t level, char const *source_file, int source_line, Args const &... format_args) noexcept {
        let message = log_message{level, source_file, source_line, format_args...};
        message_queue().push(message);
    }
};

}

#define LOG_TRACE(...) BOOST_LOG_TRIVIAL(trace) << fmt::format(__VA_ARGS__)
#define LOG_DEBUG(...) BOOST_LOG_TRIVIAL(debug) << fmt::format(__VA_ARGS__)
#define LOG_INFO(...) BOOST_LOG_TRIVIAL(info) << fmt::format(__VA_ARGS__)
#define LOG_WARNING(...) BOOST_LOG_TRIVIAL(warning) << fmt::format(__VA_ARGS__)
#define LOG_ERROR(...) BOOST_LOG_TRIVIAL(error) << fmt::format(__VA_ARGS__)
#define LOG_FATAL(...) BOOST_LOG_TRIVIAL(fatal) << fmt::format(__VA_ARGS__); std::abort()

