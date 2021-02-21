// Copyright Take Vos 2019-2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "unfair_mutex.hpp"
#include <atomic>
#include <vector>
#include <functional>
#include <bit>

namespace tt {

enum class system_status_type : uint32_t {
    log_level_debug = 0x01,
    log_level_info = 0x02,
    log_level_statistics = 0x04,
    log_level_trace = 0x08,
    log_level_warning = 0x10,
    log_level_audit = 0x20,
    log_level_error = 0x40,
    log_level_fatal = 0x80,

    shutdown = 0x1'00,
    logger = 0x2'00,
    statistics = 0x4'00,
};

[[nodiscard]] constexpr system_status_type operator&(system_status_type const &lhs, system_status_type const &rhs) noexcept
{
    return static_cast<system_status_type>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

[[nodiscard]] constexpr system_status_type operator|(system_status_type const &lhs, system_status_type const &rhs) noexcept
{
    return static_cast<system_status_type>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

[[nodiscard]] constexpr system_status_type operator~(system_status_type const &rhs) noexcept
{
    return static_cast<system_status_type>(~static_cast<uint32_t>(rhs));
}

[[nodiscard]] constexpr uint8_t to_log_level(system_status_type const &rhs) noexcept
{
    return static_cast<uint8_t>(rhs);
}

/** The status of the system, as an atomic value.
 * The status of the system is used as a bit field, so that multiple status flags can
 * be checked with a single read.
 *
 * The system status should only be written to when holding the system_status_mutex.
 */
inline std::atomic<system_status_type> system_status = system_status_type{0};

namespace detail {

/** A list of deinit function to be called on shutdown.
 */
inline std::vector<std::function<void()>> system_status_deinit_list;

/** Mutex to be held when writing to system_status or accessing system_status_deinit_list.
 * The system status is also an atomic variable so that reads on system_status
 * without holding the mutex is still possible.
 */
inline unfair_mutex system_status_mutex;
static_assert(decltype(system_status)::is_always_lock_free);

template<typename InitFunc, typename DeinitFunc>
tt_no_inline bool system_status_start_subsystem(system_status_type subsystem, InitFunc init_function, DeinitFunc deinit_function)
{
    tt_axiom(std::popcount(static_cast<uint32_t>(subsystem)) == 1);
    ttlet lock = std::scoped_lock(system_status_mutex);

    ttlet current_state = system_status.load();

    if (static_cast<bool>(current_state & system_status_type::shutdown)) {
        return false;
    }

    if (static_cast<bool>(current_state & subsystem)) {
        return true;
    }

    system_status_deinit_list.emplace_back(std::forward<DeinitFunc>(deinit_function));

    std::forward<InitFunc>(init_function)();

    system_status.store(current_state | subsystem);
    return true;
}

} // namespace detail

/** Set the log level of the system.
 * @param log_level The log level to set in the system_status variable.
 */
inline void system_status_set_log_level(uint8_t log_level) noexcept
{
    ttlet lock = std::scoped_lock(detail::system_status_mutex);

    auto current_status = static_cast<uint32_t>(system_status.load());
    current_status >>= 8;
    current_status <<= 8;
    current_status |= log_level;
    system_status.store(static_cast<system_status_type>(current_status));
}

/** Start a sub-system.
 * Initialize a subsystem. The subsystem is not started if the following conditions are true:
 *  - System shutdown is in progress.
 *  - The subsystem is already initialized.
 *
 * This will also register the deinit function to be called on system shutdown.
 *
 * @param subsystem The flag of the subsystem to enable.
 * @param init_function The init function to call to initialize the subsystem
 * @param deinit_function the deinit function to call when shutting down the system.
 * @return true if the sub-system is initialized, false if the system is shutting down.
 */
template<typename InitFunc, typename DeinitFunc>
bool system_status_start_subsystem(system_status_type subsystem, InitFunc init_function, DeinitFunc deinit_function)
{
    tt_axiom(std::popcount(static_cast<uint32_t>(subsystem)) == 1);

    if (!static_cast<bool>(system_status.load(std::memory_order_relaxed) & subsystem)) {
        [[unlikely]] return detail::system_status_start_subsystem(subsystem, init_function, deinit_function);
    } else {
        // Subsystem is already running.
        return true;
    }
}

/** Shutdown the system.
 * This will shutdown all the registered deinit functions.
 *
 * Any attempts at registering deinit functions after this call
 * will fail and the deinit function will be called directly.
 */
inline void system_status_shutdown() noexcept
{
    detail::system_status_mutex.lock();
    system_status = system_status | system_status_type::shutdown;

    while (!detail::system_status_deinit_list.empty()) {
        auto deinit = std::move(detail::system_status_deinit_list.back());
        detail::system_status_deinit_list.pop_back();

        detail::system_status_mutex.unlock();
        deinit();
        detail::system_status_mutex.lock();
    }
    detail::system_status_mutex.unlock();
}

} // namespace tt
