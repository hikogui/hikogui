// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "source_location.hpp"
#include <fmt/format.h>

namespace tt {

[[noreturn]] void _throw_operation_error(tt::source_location source_location, std::string message);
[[noreturn]] void _throw_overflow_error(tt::source_location source_location, std::string message);
[[noreturn]] void _throw_parse_error(tt::source_location source_location, std::string message);

template<typename... Args>
[[noreturn]] void throw_operation_error(tt::source_location source_location, char const *str, Args const &... args)
{
    _throw_operation_error(source_location, fmt::format(str, args...));
}

template<typename... Args>
[[noreturn]] void throw_overflow_error(tt::source_location source_location, char const *str, Args const &... args)
{
    _throw_overflow_error(source_location, fmt::format(str, args...));
}

template<typename... Args>
[[noreturn]] void throw_parse_error(tt::source_location source_location, char const *str, Args const &... args)
{
    _throw_parse_error(source_location, fmt::format(str, args...));
}

#define tt_throw_operation_error(...) throw_operation_error(tt::source_location(__LINE__, 0, __FILE__, __func__), __VA_ARGS__);
#define tt_throw_overflow_error(...) throw_overflow_error(tt::source_location(__LINE__, 0, __FILE__, __func__), __VA_ARGS__);
#define tt_throw_parse_error(...) throw_parse_error(tt::source_location(__LINE__, 0, __FILE__, __func__), __VA_ARGS__);

} // namespace tt