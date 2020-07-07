// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/logger.hpp"
#include "ttauri/trace.hpp"
#include "ttauri/cpu_utc_clock.hpp"
#include "ttauri/globals.hpp"
#include "ttauri/required.hpp"
#include "ttauri/URL.hpp"
#include "ttauri/strings.hpp"
#include "ttauri/thread.hpp"
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
