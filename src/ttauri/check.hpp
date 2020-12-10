// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "error_info.hpp"
#include "source_location.hpp"
#include "exception.hpp"
#include "utils.hpp"

namespace tt {

#define tt_parse_check(expression, ...) \
    do { \
        if (!(expression)) { \
            ::tt::error_info(::tt::source_location(__LINE__, 0, __FILE__, __func__)); \
            if constexpr (::tt::nr_arguments(__VA_ARGS__) == 0) { \
                throw ::tt::parse_error(#expression); \
            } else { \
                throw ::tt::parse_error(__VA_ARGS__); \
            } \
        } \
    } while (false)

#define tt_hresult_check(expression) \
    ([](HRESULT result) { \
        if (FAILED(result)) { \
            ::tt::error_info(tt::source_location(__LINE__, 0, __FILE__, __func__)); \
            throw ::tt::io_error("Call to '{}' failed with {:08x}", #expression, result); \
        } \
        return result; \
    }(expression))

}
