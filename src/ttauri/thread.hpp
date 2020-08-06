// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include <thread>
#include <string_view>
#include <functional>
#include <atomic>

namespace tt {

/** Set the name of the current thread.
 * This function will set the name of the thread so that it is available
 * by the operating system and debugger.
 *
 * Every thread should call this function exactly once.
 *
 * This function will also initialize the `current_thread_id`
 */
void set_thread_name(std::string_view name);

bool is_main_thread();

void run_on_main_thread(std::function<void()> f);

/** The current_thread_id of a thread.
 * This thread_id may not equal to the operating system's thread id.
 * The current_thread_id should never be zero.
 *
 * The current_thread_id is initialized when calling `set_thread_name()`.
 */
inline thread_local uint32_t current_thread_id = 0;

}

