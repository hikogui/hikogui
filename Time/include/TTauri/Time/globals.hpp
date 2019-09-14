// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "TTauri/Time/sync_clock.hpp"
#include "TTauri/Time/cpu_counter_clock.hpp"
#include "TTauri/Time/hires_utc_clock.hpp"
#include "TTauri/Required/URL.hpp"
#include "TTauri/Required/required.hpp"
#include <fmt/format.h>
#include <memory>
#include <vector>
#include <string>
#include <optional>

namespace TTauri {


struct TimeGlobals;
inline TimeGlobals *Time_globals = nullptr;

struct TimeGlobals {
private:
    std::optional<std::string> time_zone_error_message;

public:
    date::time_zone const *time_zone = nullptr;

    TimeGlobals(URL tzdata_location);
    ~TimeGlobals();
    TimeGlobals(TimeGlobals const &) = delete;
    TimeGlobals &operator=(TimeGlobals const &) = delete;
    TimeGlobals(TimeGlobals &&) = delete;
    TimeGlobals &operator=(TimeGlobals &&) = delete;

    std::optional<std::string> read_message(void) noexcept;
};

}
