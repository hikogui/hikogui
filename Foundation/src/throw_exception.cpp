// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/throw_exception.hpp"
#include "TTauri/Foundation/exceptions.hpp"

namespace TTauri {

[[noreturn]] void _throw_invalid_operation_error(char const *source_file, int source_line, std::string message)
{
    throw invalid_operation_error("{}", message).log(source_file, source_line);
}

[[noreturn]] void _throw_overflow_error(char const *source_file, int source_line, std::string message)
{
    throw overflow_error("{}", message).log(source_file, source_line);
}

[[noreturn]] void _throw_parse_error(char const* source_file, int source_line, std::string message)
{
    throw parse_error("{}", message).log(source_file, source_line);
}


}