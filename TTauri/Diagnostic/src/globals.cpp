// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Diagnostic/globals.hpp"
#include "TTauri/Diagnostic/logger.hpp"
#include "TTauri/Time/globals.hpp"
#include "TTauri/Required/globals.hpp"

namespace TTauri {

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