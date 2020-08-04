// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <thread>
#include <string_view>
#include <functional>
#include <atomic>

namespace tt {

void set_thread_name(std::string_view name);

bool is_main_thread();

void run_on_main_thread(std::function<void()> f);

inline std::atomic<uint32_t> thread_id_count = 0;

inline thread_local uint32_t current_thread_id = 0;

}

