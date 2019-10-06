// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri {

static void assert_logger(char const *source_file, int source_line, char const *expression)
{
    logger.log<log_level::Assert>(cpu_counter_clock::now(), "{}", expression, source_code_ptr(source_file, source_line));
}

DiagnosticGlobals::DiagnosticGlobals()
{
    required_assert(Required_globals != nullptr);
    required_assert(Time_globals != nullptr);
    required_assert(Diagnostic_globals == nullptr);
    Diagnostic_globals = this;

    logger.startLogging();
    logger.startStatisticsLogging();
}

DiagnosticGlobals::~DiagnosticGlobals()
{
    // This will log all current counters then all
    // messages that are left in the queue..
    logger.stopStatisticsLogging();
    logger.stopLogging();

    required_assert(Diagnostic_globals == this);
    Diagnostic_globals = nullptr;
}

}