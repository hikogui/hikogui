// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "hires_utc_clock.hpp"
#include "application.hpp"
#include "logger.hpp"
#include <fmt/ostream.h>
#include <fmt/format.h>

namespace tt {

using namespace std::chrono_literals;

std::string format_engineering(hires_utc_clock::duration duration)
{
    if (duration >= 1s) {
        return fmt::format("{:.3g} s ", static_cast<double>(duration / 1ns) / 1'000'000'000);
    } else if (duration >= 1ms) {
        return fmt::format("{:.3g} ms", static_cast<double>(duration / 1ns) / 1'000'000);
    } else if (duration >= 1us) {
        return fmt::format("{:.3g} us", static_cast<double>(duration / 1ns) / 1'000);
    } else {
        return fmt::format("{:.3g} ns", static_cast<double>(duration / 1ns));
    }
}



} // namespace tt