// Copyright 2019 Pokitec
// All rights reserved.

#include "logger.hpp"
#include "trace.hpp"
#include "cpu_utc_clock.hpp"
#include "globals.hpp"
#include "required.hpp"
#include "URL.hpp"
#include "strings.hpp"
#include "thread.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace tt {

std::string get_last_error_message()
{
    return strerror(errno);
}

}
