// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <atomic>
#include <cstdint>
#include <string_view>

namespace tt {

enum class log_level : uint8_t {
    debug = 0x01,
    info = 0x02,
    statistics = 0x04,
    trace = 0x08,
    audit = 0x10,
    warning = 0x20,
    error = 0x40,
    fatal = 0x80
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
[[nodiscard]] log_level make_log_level(log_level user_level) noexcept;

char const *to_const_string(log_level level) noexcept;

int command_line_argument_to_log_level(std::string_view str);

inline std::atomic<log_level> log_level_global = make_log_level(log_level::debug);

}
