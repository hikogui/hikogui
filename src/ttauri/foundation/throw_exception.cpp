// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/throw_exception.hpp"
#include "ttauri/foundation/exceptions.hpp"

namespace tt {

[[noreturn]] void _throw_invalid_operation_error(char const *source_file, int source_line, std::string message)
{
    throw invalid_operation_error("{}", message).log(source_file, source_line);
}

[[noreturn]] void _throw_math_error(char const *source_file, int source_line, std::string message)
{
    throw math_error("{}", message).log(source_file, source_line);
}

[[noreturn]] void _throw_parse_error(char const* source_file, int source_line, std::string message)
{
    throw parse_error("{}", message).log(source_file, source_line);
}


}