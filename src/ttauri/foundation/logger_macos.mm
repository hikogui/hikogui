// Copyright 2019 Pokitec
// All rights reserved.

#include "ttauri/foundation/logger.hpp"
#include "ttauri/foundation/trace.hpp"
#include "ttauri/foundation/cpu_utc_clock.hpp"
#include "ttauri/foundation/globals.hpp"
#include "ttauri/foundation/required.hpp"
#include "ttauri/foundation/URL.hpp"
#include "ttauri/foundation/strings.hpp"
#include "ttauri/foundation/thread.hpp"
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
