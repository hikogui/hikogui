// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "cast.hpp"
#include <atomic>
#include <type_traits>
#include <bit>

hi_warning_push();
// C26490: Don't use reinterpret_cast
// Need it for allow the use of enum in an atomic operation.
hi_warning_ignore_msvc(26490);

namespace hi::inline v1 {
enum class global_state_type : uint64_t {
    log_debug = 0x01,
    log_info = 0x02,
    log_statistics = 0x04,
    log_trace = 0x08,
    log_audit = 0x10,
    log_warning = 0x20,
    log_error = 0x40,
    log_fatal = 0x80,

    log_mask = log_debug | log_info | log_statistics | log_trace | log_audit | log_warning | log_error | log_fatal,

    log_level_default = log_audit | log_error | log_fatal,
    log_level_fatal = log_audit | log_fatal,
    log_level_error = log_trace | log_error | log_level_fatal,
    log_level_warning = log_warning | log_level_error,
    log_level_info = log_info | log_statistics | log_level_warning,
    log_level_debug = log_debug | log_level_info,

    log_is_running = 0x1'00,
    time_stamp_utc_is_running = 0x2'00,

    system_is_running = 0x1'000000'00,
    system_is_shutting_down = 0x2'000000'00,
    system_mask = system_is_running | system_is_shutting_down,
};

[[nodiscard]] constexpr global_state_type operator|(global_state_type lhs, global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(to_underlying(lhs) | to_underlying(rhs));
}

[[nodiscard]] constexpr global_state_type operator&(global_state_type lhs, global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(to_underlying(lhs) & to_underlying(rhs));
}

[[nodiscard]] constexpr global_state_type operator~(global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(~to_underlying(rhs));
}

[[nodiscard]] constexpr bool to_bool(global_state_type rhs) noexcept
{
    return to_bool(to_underlying(rhs));
}

[[nodiscard]] constexpr bool is_system_running(global_state_type rhs) noexcept
{
    return (rhs & global_state_type::system_mask) == global_state_type::system_is_running;
}

[[nodiscard]] constexpr bool is_system_shutting_down(global_state_type rhs) noexcept
{
    return to_bool(rhs & global_state_type::system_is_shutting_down);
}

} // namespace hi::inline v1

template<>
struct std::atomic<hi::global_state_type> {
    using value_type = hi::global_state_type;
    using atomic_type = std::atomic<underlying_type_t<value_type>>;
    atomic_type v;

    static constexpr bool is_always_lock_free = atomic_type::is_always_lock_free;

    constexpr atomic() noexcept = default;
    atomic(atomic const&) = delete;
    atomic(atomic&&) = delete;
    atomic& operator=(atomic const&) = delete;
    atomic& operator=(atomic&&) = delete;

    constexpr atomic(value_type desired) noexcept : v(to_underlying(desired)) {}

    [[nodiscard]] bool is_lock_free() const noexcept
    {
        return v.is_lock_free();
    }

    void store(value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return v.store(hi::to_underlying(desired), order);
    }

    [[nodiscard]] value_type load(std::memory_order order = std::memory_order::seq_cst) const noexcept
    {
        return static_cast<value_type>(v.load(order));
    }

    [[nodiscard]] value_type exchange(value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.exchange(hi::to_underlying(desired), order));
    }

    [[nodiscard]] bool
    compare_exchange_weak(value_type& expected, value_type desired, std::memory_order success, std::memory_order failure) noexcept
    {
        return v.compare_exchange_weak(
            reinterpret_cast<underlying_type_t<value_type>&>(expected), hi::to_underlying(desired), success, failure);
    }

    [[nodiscard]] bool
    compare_exchange_weak(value_type& expected, value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return compare_exchange_weak(expected, desired, order, order);
    }

    [[nodiscard]] bool compare_exchange_strong(
        value_type& expected,
        value_type desired,
        std::memory_order success,
        std::memory_order failure) noexcept
    {
        return v.compare_exchange_weak(
            reinterpret_cast<underlying_type_t<value_type>&>(expected), hi::to_underlying(desired), success, failure);
    }

    [[nodiscard]] bool compare_exchange_strong(
        value_type& expected,
        value_type desired,
        std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, order, order);
    }

    value_type fetch_and(value_type arg, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.fetch_and(hi::to_underlying(arg), order));
    }

    value_type fetch_or(value_type arg, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.fetch_or(hi::to_underlying(arg), order));
    }

    operator value_type() const noexcept
    {
        return load();
    }

    value_type operator=(value_type desired) noexcept
    {
        store(desired);
        return desired;
    }

    value_type operator|=(value_type arg) noexcept
    {
        return fetch_or(arg) | arg;
    }

    value_type operator&=(value_type arg) noexcept
    {
        return fetch_and(arg) & arg;
    }
};

namespace hi::inline v1 {

/** The global state of the hikogui framework.
 *
 * This variable contains state in use by multiple systems
 * inside hikogui. This is done so that this variable is likely
 * to be in a cache line and may stay in a register.
 *
 * In many cases using std::memory_order::relaxed loads are enough of a
 * guarantee to read this variable.
 */
inline std::atomic<global_state_type> global_state = global_state_type::log_level_default;

[[nodiscard]] inline bool is_system_running() noexcept
{
    return is_system_running(global_state.load(std::memory_order::relaxed));
}

[[nodiscard]] inline bool is_system_shutting_down() noexcept
{
    return is_system_shutting_down(global_state.load(std::memory_order::relaxed));
}

inline void set_log_level(global_state_type log_level) noexcept
{
    // Only the log_* bits should be set.
    hi_assert(not to_bool(log_level & ~global_state_type::log_mask));

    // First enable bits, then disable bits.
    global_state |= log_level;
    global_state &= (~global_state_type::log_mask | log_level);
}

/** Disable a subsystem.
 *
 * @param subsystem The subsystem to disable.
 * @param order Memory order to use on the global_state variable.
 * @return True if the subsystem was enabled.
 */
inline bool global_state_disable(global_state_type subsystem, std::memory_order order = std::memory_order::seq_cst) noexcept
{
    hi_assert(std::popcount(to_underlying(subsystem)) == 1);
    return to_bool(global_state.fetch_and(~subsystem, order) & subsystem);
}

/** Enable a subsystem.
 *
 * @param subsystem The subsystem to disable.
 * @param order Memory order to use on the global_state variable.
 * @return True if the subsystem was enabled.
 */
inline bool global_state_enable(global_state_type subsystem, std::memory_order order = std::memory_order::seq_cst) noexcept
{
    hi_assert(std::popcount(to_underlying(subsystem)) == 1);
    return to_bool(global_state.fetch_or(subsystem, order) & subsystem);
}

} // namespace hi::inline v1

hi_warning_pop();
