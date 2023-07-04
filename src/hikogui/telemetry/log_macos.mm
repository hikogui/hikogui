// Copyright 2019 Pokitec
// All rights reserved.

#include "log.hpp"
#include "trace.hpp"
#include "cpu_utc_clock.hpp"
#include "strings.hpp"
#include "utility/module.hpp"
#include <format>
#include <exception>
#include <memory>
#include <iostream>
#include <ostream>
#include <chrono>

namespace hi::inline v1 {

std::string get_last_error_message()
{
    return strerror(errno);
}

}
