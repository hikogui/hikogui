// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "cast.hpp"
#include <atomic>
#include <type_traits>

namespace tt {

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

    logger_is_running = 0x1'00,

    system_is_running = 0x1'000000'00,
    system_is_shutting_down = 0x2'000000'00,
    system_mask = system_is_running | system_is_shutting_down,
};

[[nodiscard]] constexpr global_state_type operator|(global_state_type lhs, global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(underlying_cast(lhs) | underlying_cast(rhs));
}

[[nodiscard]] constexpr global_state_type operator&(global_state_type lhs, global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(underlying_cast(lhs) & underlying_cast(rhs));
}

[[nodiscard]] constexpr global_state_type operator~(global_state_type rhs) noexcept
{
    return static_cast<global_state_type>(~underlying_cast(rhs));
}

[[nodiscard]] constexpr bool to_bool(global_state_type rhs) noexcept
{
    return static_cast<bool>(underlying_cast(rhs));
}

[[nodiscard]] constexpr bool is_system_running(global_state_type rhs) noexcept
{
    return (rhs & global_state_type::system_mask) == global_state_type::system_is_running;
}

[[nodiscard]] constexpr bool is_system_shutting_down(global_state_type rhs) noexcept
{
    return to_bool(global_state_type::system_mask & global_state_type::system_is_shutting_down);
}

} // namespace tt

namespace std {

template<>
struct atomic<tt::global_state_type> {
    using value_type = tt::global_state_type;
    using atomic_type = atomic<underlying_type_t<value_type>>;
    atomic_type v;

    static constexpr bool is_always_lock_free = atomic_type::is_always_lock_free;

    constexpr atomic() noexcept = default;
    constexpr atomic(value_type desired) noexcept : v(underlying_cast(desired)) {}
    constexpr atomic(atomic const &) = delete;

    atomic &operator=(const atomic &) = delete;

    [[nodiscard]] bool is_lock_free() const noexcept
    {
        return v.is_lock_free();
    }

    void store(value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return v.store(tt::underlying_cast(desired), order);
    }

    [[nodiscard]] value_type load(std::memory_order order = std::memory_order::seq_cst) const noexcept
    {
        return static_cast<value_type>(v.load(order));
    }

    [[nodiscard]] value_type exchange(value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.exchange(tt::underlying_cast(desired), order));
    }

    [[nodiscard]] bool
    compare_exchange_weak(value_type &expected, value_type desired, std::memory_order success, std::memory_order failure) noexcept
    {
        return v.compare_exchange_weak(
            reinterpret_cast<underlying_type_t<value_type> &>(expected), tt::underlying_cast(desired), success, failure);
    }

    [[nodiscard]] bool
    compare_exchange_weak(value_type &expected, value_type desired, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return compare_exchange_weak(expected, desired, order, order);
    }

    [[nodiscard]] bool compare_exchange_strong(
        value_type &expected,
        value_type desired,
        std::memory_order success,
        std::memory_order failure) noexcept
    {
        return v.compare_exchange_weak(
            reinterpret_cast<underlying_type_t<value_type> &>(expected), tt::underlying_cast(desired), success, failure);
    }

    [[nodiscard]] bool compare_exchange_strong(
        value_type &expected,
        value_type desired,
        std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return compare_exchange_strong(expected, desired, order, order);
    }

    value_type fetch_and(value_type arg, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.fetch_and(tt::underlying_cast(arg), order));
    }

    value_type fetch_or(value_type arg, std::memory_order order = std::memory_order::seq_cst) noexcept
    {
        return static_cast<value_type>(v.fetch_or(tt::underlying_cast(arg), order));
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

} // namespace std

namespace tt {

/** The global state of the ttauri framework.
 *
 * This variable contains state in use by multiple systems
 * inside ttauri. This is done so that this variable is likely
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

[[nodiscard]] inline void set_log_level(global_state_type log_level) noexcept
{
    // Only the log_* bits should be set.
    tt_axiom(not to_bool(log_level & ~global_state_type::log_mask));

    // First enable bits, then disable bits.
    global_state |= log_level;
    global_state &= (~global_state_type::log_mask | log_level);
}

} // namespace tt
