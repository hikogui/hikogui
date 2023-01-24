// Copyright Take Vos 2023.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file terminate.hpp Utilities for throwing exceptions and terminating the application.
 */

#include "utility/module.hpp"
#include <exception>
#include <stdexcept>
#include <atomic>
#include <bit>
#include <format>

#pragma once

namespace hi { inline namespace v1 {

/** The old terminate handler.
 *
 * This is the handler returned by `std::set_terminate()`.
 */
inline std::terminate_handler old_terminate_handler;

/** The HikoGUI terminate handler.
 *
 * This handler will print an error message on the console or pop-up a dialogue box.
 *
 * @note Use `hi_set_terminate_message()` to set a message.
 */
[[noreturn]] void terminate_handler() noexcept;

}} // namespace hi::v1
