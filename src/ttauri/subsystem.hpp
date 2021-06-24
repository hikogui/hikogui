// Copyright Take Vos 2019-2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "unfair_recursive_mutex.hpp"
#include "type_traits.hpp"
#include <atomic>
#include <vector>
#include <functional>
#include <bit>
#include <type_traits>
#include <mutex>

namespace tt {
namespace detail {

/** The status of the system, as an atomic value.
 * The status of the system is used as a bit field, so that multiple status flags can
 * be checked with a single read.
 *
 * The system status should only be written to when holding the system_status_mutex.
 */
enum class system_status_type { not_started, running, shutdown };

inline system_status_type system_status = system_status_type::not_started;

/** A list of deinit function to be called on shutdown.
 */
inline std::vector<void (*)()> subsystem_deinit_list;

/** Mutex to be held when writing to system_status or accessing system_status_deinit_list.
 * The system status is also an atomic variable so that reads on system_status
 * without holding the mutex is still possible.
 */
inline unfair_recursive_mutex subsystem_mutex;

template<typename T> requires(is_atomic_v<T>)
tt_no_inline typename T::value_type start_subsystem(
    T &check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
{
    ttlet lock = std::scoped_lock(subsystem_mutex);

    auto old_value = check_variable.load(std::memory_order::acquire);
    if (old_value != off_value) {
        // In the short time before the lock the subsystem became available.
        return old_value;
    }

    if (system_status != system_status_type::running) {
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

} // namespace detail

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @param check_variable The variable to check before initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
template<typename T>
requires(is_atomic_v<T>) typename T::value_type start_subsystem(
    T &check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
{
    // We can do a relaxed load, if:
    //  - off_value, then we will lock before writing check_variable and memory order will be guaranteed
    //  - not off_value, The system is started. If the subsystem is turning off we can't deal with that anyway.
    auto old_value = check_variable.load(std::memory_order::relaxed);
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
 * If the subsystem was unable to be started because of the previous conditions then
 * this function will std::terminate() the program.
 * 
 * This will also register the deinit function to be called on system shutdown.
 *
 * @param check_variable The variable to check before initializing.
 * @param off_value The value of the check_variable when the subsystem is off.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return return value from the init_function; off_value if the system is shutting down.
 */
template<typename T>
requires(is_atomic_v<T>) typename T::value_type start_subsystem_or_terminate(
    T &check_variable,
    typename T::value_type off_value,
    typename T::value_type (*init_function)(),
    void (*deinit_function)())
{
    auto old_value = check_variable.load(std::memory_order::acquire);
    if (old_value == off_value) {
        auto tmp = detail::start_subsystem(check_variable, off_value, init_function, deinit_function);
        tt_assert(tmp != off_value);
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
 * @param deinit_function the deinit function to call.
 */
inline void stop_subsystem(void (*deinit_function)())
{
    ttlet lock = std::scoped_lock(detail::subsystem_mutex);

    std::erase(detail::subsystem_deinit_list, deinit_function);
    return deinit_function();
}

/** Start the system.
 * Subsystems will only initialize once the system is started.
 */
inline void start_system() noexcept
{
    ttlet lock = std::scoped_lock(detail::subsystem_mutex);
    detail::system_status = detail::system_status_type::running;
}

[[nodiscard]] inline bool system_shutting_down() noexcept
{
    return detail::system_status == detail::system_status_type::shutdown;
}

/** Shutdown the system.
 * This will shutdown all the registered deinit functions.
 *
 * Any attempts at registering deinit functions after this call
 * will fail and the deinit function will be called directly.
 */
inline void shutdown_system() noexcept
{
    detail::subsystem_mutex.lock();
    detail::system_status = detail::system_status_type::shutdown;

    while (!detail::subsystem_deinit_list.empty()) {
        auto deinit = std::move(detail::subsystem_deinit_list.back());
        detail::subsystem_deinit_list.pop_back();

        detail::subsystem_mutex.unlock();
        deinit();
        detail::subsystem_mutex.lock();
    }
    detail::subsystem_mutex.unlock();
}

} // namespace tt
