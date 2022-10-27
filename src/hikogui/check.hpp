// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "exception.hpp"
#include <format>

namespace hi::inline v1 {

#define hi_parse_check(expression, message, ...) \
    do { \
        if (!(expression)) { \
            if constexpr (__VA_OPT__(not ) false) { \
                throw ::hi::parse_error(std::format(message __VA_OPT__(, ) __VA_ARGS__)); \
            } else { \
                throw ::hi::parse_error(message); \
            } \
        } \
    } while (false)

#define hi_hresult_check(expression) \
    ([](HRESULT result) { \
        if (FAILED(result)) { \
            throw ::hi::io_error(std::format("Call to '{}' failed with {:08x}", #expression, result)); \
        } \
        return result; \
    }(expression))

} // namespace hi::inline v1
