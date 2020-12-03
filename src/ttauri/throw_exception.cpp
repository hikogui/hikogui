// Copyright 2019 Pokitec
// All rights reserved.

#include "throw_exception.hpp"
#include "exception.hpp"
#include "error_info.hpp"
#include "source_location.hpp"

namespace tt {

[[noreturn]] void _throw_operation_error(tt::source_location source_location, std::string message)
{
    auto info = error_info(source_location);
    throw operation_error(message);
}

[[noreturn]] void _throw_overflow_error(tt::source_location source_location, std::string message)
{
    auto info = error_info(source_location);
    throw std::overflow_error(message);
}

[[noreturn]] void _throw_parse_error(tt::source_location source_location, std::string message)
{
    auto info = error_info(source_location);
    throw parse_error(message);
}


}