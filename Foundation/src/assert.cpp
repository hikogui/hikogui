// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/logger.hpp"

namespace TTauri {

[[noreturn]] void assertIsFatal(const char *source_file, int source_line, const char *message)
{
    logger.log<log_level::Fatal>(
        cpu_counter_clock::now(),
        "{}", message,
        source_code_ptr(source_file, source_line)
    );
    // logger.log<log_level::Fatal> will terminate, do it twice to shut up compiler.
    std::terminate();
}

void assertIsLogged(const char *source_file, int source_line, const char *message)
{
    logger.log<log_level::Assert>(
        cpu_counter_clock::now(),
        "{}", message,
        source_code_ptr(source_file, source_line)
    );
}

}