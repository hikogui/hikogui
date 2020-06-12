// Copyright 2019 Pokitec
// All rights reserved.

#include "TTauri/Foundation/logger.hpp"
#include "TTauri/Foundation/trace.hpp"
#include "TTauri/Foundation/cpu_utc_clock.hpp"
#include "TTauri/Foundation/globals.hpp"
#include "TTauri/Foundation/required.hpp"
#include "TTauri/Foundation/URL.hpp"
#include "TTauri/Foundation/strings.hpp"
#include "TTauri/Foundation/thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace tt {

std::string getLastErrorMessage()
{
    return strerror(errno);
}

}
