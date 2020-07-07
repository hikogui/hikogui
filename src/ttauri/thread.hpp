// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "ttauri/required.hpp"
#include <thread>
#include <string_view>
#include <functional>

namespace tt {

void set_thread_name(std::string_view name);

bool is_main_thread();

void run_on_main_thread(std::function<void()> f);


}

