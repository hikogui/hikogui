// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "assert.hpp"
#include "cast.hpp"
#include <atomic>
#include <cstdint>

namespace hi::inline v1 {

/** Counts how many times a critical section was idle.
 *
 * A reader thread.
 * ```cpp
 * idle_count.read_lock();
 * ... read protected data ...
 * idle_count.read_unlock();
 * ```
 *
 * A write thread.
 * ```cpp
 * idle_count.write_lock();
 * ... write protected data ...
 * auto version = idle_count.write_unlock();
 *
 * ... wait some time ...
 * 
 * if (idle_count.is_seen(version)) {
 *   // All threads now see the new data.
 *   ... Delete old data ...
 * }
 * ```
 */
class wfree_idle_count {
public:
    constexpr wfree_idle_count() = default;
    ~wfree_idle_count() = default;
    wfree_idle_count(wfree_idle_count const&) = delete;
    wfree_idle_count(wfree_idle_count&&) = delete;
    wfree_idle_count& operator=(wfree_idle_count const&) = delete;
    wfree_idle_count& operator=(wfree_idle_count&&) = delete;

    /** Check if the critical section is locked.
     *
     * @note This is only reliably `true` when inside a critical section.
     */
    [[nodiscard]] hi_force_inline bool is_locked() const noexcept
    {
        return to_bool(_lock_count.load(std::memory_order::relaxed));
    }

    /** Start the critical section for reading.
     *
     * @note lock is allowed to be called reentered.
     */
    hi_force_inline void read_lock() noexcept
    {
        auto lock_count = _lock_count.fetch_add(1, std::memory_order::acquire);
        hi_axiom(lock_count != std::numeric_limits<uint32_t>::max());
    }

    /** Start the critical section for writing.
     *
     * @note lock is allowed to be called reentered.
     */
    hi_force_inline void write_lock() noexcept
    {
        auto lock_count = _lock_count.fetch_add(1, std::memory_order::acquire);
        hi_axiom(lock_count != std::numeric_limits<uint32_t>::max());
    }

    /** End the critical section.
     *
     * @note It is undefined behavior to call unlock() when not holding the lock.
     */
    hi_force_inline void read_unlock() noexcept
    {
        auto lock_count = _lock_count.fetch_sub(1, std::memory_order::release);
        hi_axiom(lock_count != 0);
        if (lock_count == 1) {
            // No one is locking, increment the idle count.
            _idle_count.fetch_add(1, std::memory_order::relaxed);
        }
    }

    /** End the critical section.
     *
     * @note It is undefined behavior to call unlock() when not holding the lock.
     * @return A opaque version number used to determine if the write is seen by all threads.
     */
    [[nodiscard]] hi_force_inline uint32_t write_unlock() noexcept
    {
        // The idle count does not change while the lock_count is non-zero.
        auto version = _idle_count.load(std::memory_order::release);

        auto lock_count = _lock_count.fetch_sub(1, std::memory_order::release);
        hi_axiom(lock_count != 0);
        if (lock_count == 1) {
            // No one is locking, increment the idle count.
            _idle_count.fetch_add(1, std::memory_order::relaxed);
        }

        return version;
    }

    /** Check if all threads are seeing the updated data.
     *
     * @param version The version number returned by `write_unlock()`.
     * @param memory_order The memory order used (default: release).
     * @return true when all threads will see the updated data.
     */
    [[nodiscard]] hi_force_inline bool is_seen(uint32_t version) const noexcept
    {
        auto current_count = wide_cast<uint64_t>(_idle_count.load(std::memory_order::acquire));
        hilet carry = truncate<uint64_t>(current_count < version);
        current_count |= carry << 32;

        hi_axiom(current_count >= version);
        return to_bool(current_count - version);
    }

private:
    std::atomic<uint32_t> _lock_count = 0;
    std::atomic<uint32_t> _idle_count = 0;
};

} // namespace hi::inline v1
