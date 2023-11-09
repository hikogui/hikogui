// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/subsystem.hpp Functions for starting and stopping subsystems.
 * @ingroup concurrency
 */

#pragma once

#include "../utility/utility.hpp"
#include "unfair_recursive_mutex.hpp"
#include "global_state.hpp"
#include "../macros.hpp"
#include <atomic>
#include <vector>
#include <functional>
#include <bit>
#include <type_traits>
#include <mutex>

hi_export_module(hikogui.concurrency.subsystem);


hi_export namespace hi { inline namespace v1 {
namespace detail {

/** A list of deinit function to be called on shutdown.
 */
hi_inline std::vector<void (*)()> subsystem_deinit_list;

/** Mutex to be held when writing to system_status or accessing system_status_deinit_list.
 * The system status is also an atomic variable so that reads on system_status
 * without holding the mutex is still possible.
 */
hi_inline unfair_recursive_mutex subsystem_mutex;

template<typename T>
hi_no_inline typename T::value_type start_subsystem(
    T& check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
    requires(is_atomic_v<T>)
{
    hi_assert_not_null(init_function);
    hi_assert_not_null(deinit_function);
    hilet lock = std::scoped_lock(subsystem_mutex);

    hilet old_value = check_variable.load(std::memory_order::acquire);
    if (old_value != off_value) {
        // In the short time before the lock the subsystem became available.
        return old_value;
    }

    if (not is_system_running()) {
        // Only when the system is running can subsystems be started.
        // otherwise they have to run in degraded mode.
        return off_value;
    }

    auto new_value = init_function();

    if (new_value != off_value) {
        subsystem_deinit_list.emplace_back(deinit_function);
        check_variable.store(new_value, std::memory_order::release);
    }

    return new_value;
}

hi_no_inline hi_inline bool start_subsystem(global_state_type state_bit, bool (*init_function)(), void (*deinit_function)())
{
    hi_assert(std::popcount(std::to_underlying(state_bit)) == 1);
    hi_assert_not_null(init_function);
    hi_assert_not_null(deinit_function);

    hilet lock = std::scoped_lock(subsystem_mutex);

    hilet old_state = global_state.load(std::memory_order::acquire);
    if (not is_system_running(old_state)) {
        // Only when the system is running can subsystems be started.
        // otherwise they have to run in degraded mode.
        return false;

    } else if (to_bool(old_state & state_bit)) {
        // In the short time before the lock the subsystem became available.
        return true;
    }

    if (init_function()) {
        subsystem_deinit_list.emplace_back(deinit_function);
        global_state_enable(state_bit, std::memory_order::release);
        return true;
    }

    return false;
}

} // namespace detail

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @ingroup concurrency
 * @param check_variable The variable to check before initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
template<typename T>
typename T::value_type start_subsystem(
    T& check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
    requires(is_atomic_v<T>)
{
    // We can do a relaxed load, if:
    //  - off_value, then we will lock before writing check_variable and memory order will be guaranteed
    //  - not off_value, The system is started. If the subsystem is turning off we can't deal with that anyway.
    hilet old_value = check_variable.load(std::memory_order::relaxed);
    if (old_value == off_value) {
        return detail::start_subsystem(check_variable, off_value, init_function, deinit_function);
    } else {
        [[likely]] return old_value;
    }
}

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @ingroup concurrency
 * @param state_bit The global state bit to check if the subsystem is already initialized.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
hi_inline bool start_subsystem(global_state_type state_bit, bool (*init_function)(), void (*deinit_function)())
{
    // We can do a relaxed load, if:
    //  - off_value, then we will lock before writing check_variable and memory order will be guaranteed
    //  - not off_value, The system is started. If the subsystem is turning off we can't deal with that anyway.
    if (not to_bool(global_state.load(std::memory_order::relaxed) & state_bit)) {
        return detail::start_subsystem(state_bit, init_function, deinit_function);
    } else {
        [[likely]] return true;
    }
}

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * If the subsystem was unable to be started because of the previous conditions then
 * this function will std::terminate() the program.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @ingroup concurrency
 * @param check_variable The variable to check before initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
template<typename T>
    requires(is_atomic_v<T>)
typename T::value_type start_subsystem_or_terminate(
    T& check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
{
    auto old_value = check_variable.load(std::memory_order::acquire);
    if (old_value == off_value) {
        auto tmp = detail::start_subsystem(check_variable, off_value, init_function, deinit_function);
        hi_assert(tmp != off_value);
        return tmp;
    } else {
        [[likely]] return old_value;
    }
}

/** Stop a sub-system.
 * De-initialize a subsystem.
 *
 * This will unregister and call the deinit function.
 *
 * @ingroup concurrency
 * @param deinit_function the deinit function to call.
 */
hi_inline void stop_subsystem(void (*deinit_function)())
{
    hi_assert_not_null(deinit_function);

    hilet lock = std::scoped_lock(detail::subsystem_mutex);

    std::erase(detail::subsystem_deinit_list, deinit_function);
    return deinit_function();
}

/** Start the system.
 * Subsystems will only initialize once the system is started.
 *
 * @ingroup concurrency
 */
hi_inline void start_system() noexcept
{
    global_state |= global_state_type::system_is_running;
}

/** Shutdown the system.
 * This will shutdown all the registered deinit functions.
 *
 * Any attempts at registering deinit functions after this call
 * will fail and the deinit function will be called directly.
 *
 * @ingroup concurrency
 */
hi_inline void shutdown_system() noexcept
{
    detail::subsystem_mutex.lock();
    global_state |= global_state_type::system_is_shutting_down;

    while (!detail::subsystem_deinit_list.empty()) {
        auto deinit = std::move(detail::subsystem_deinit_list.back());
        hi_assert_not_null(deinit);
        detail::subsystem_deinit_list.pop_back();

        detail::subsystem_mutex.unlock();
        deinit();
        detail::subsystem_mutex.lock();
    }
    detail::subsystem_mutex.unlock();
}

}} // namespace hi::v1
