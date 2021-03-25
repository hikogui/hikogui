// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "log_level.hpp"
#include "exception.hpp"

namespace tt {

/** Make a log level mask based on a user given log level.
 */
[[nodiscard]] log_level make_log_level(log_level user_level) noexcept
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

char const *to_const_string(log_level level) noexcept
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

int command_line_argument_to_log_level(std::string_view str)
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

}
