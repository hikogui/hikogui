// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include <fmt/format.h>

namespace TTauri {

[[noreturn]] void _throw_invalid_operation_error(char const *source_file, int source_line, std::string message);
[[noreturn]] void _throw_math_error(char const* source_file, int source_line, std::string message);
[[noreturn]] void _throw_parse_error(char const* source_file, int source_line, std::string message);

template<typename... Args>
[[noreturn]] void throw_invalid_operation_error(char const *source_file, int source_line, char const *str, Args &&... args) {
    _throw_invalid_operation_error(source_file, source_line, fmt::format(str, std::forward<Args>(args)...));
}

template<typename... Args>
[[noreturn]] void throw_math_error(char const* source_file, int source_line, char const* str, Args&&... args) {
    _throw_math_error(source_file, source_line, fmt::format(str, std::forward<Args>(args)...));
}

template<typename... Args>
[[noreturn]] void throw_parse_error(char const* source_file, int source_line, char const* str, Args&&... args) {
    _throw_parse_error(source_file, source_line, fmt::format(str, std::forward<Args>(args)...));
}

#define TTAURI_THROW_INVALID_OPERATION_ERROR(...) throw_invalid_operation_error(__FILE__, __LINE__, __VA_ARGS__);
#define TTAURI_THROW_MATH_ERROR(...) throw_math_error(__FILE__, __LINE__, __VA_ARGS__);
#define TTAURI_THROW_PARSE_ERROR(...) throw_parse_error(__FILE__, __LINE__, __VA_ARGS__);

#define _parse_assert(x) if (!(x)) { TTAURI_THROW_PARSE_ERROR("{}", #x ); }
#define _parse_assert2(x, ...) if (!(x)) { TTAURI_THROW_PARSE_ERROR(__VA_ARGS__); }

}