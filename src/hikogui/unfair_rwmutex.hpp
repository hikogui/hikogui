// Copyright Take Vos 2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "utility.hpp"
#include "thread.hpp"
#include "assert.hpp"
#include "dead_lock_detector.hpp"
#include <atomic>
#include <memory>
#include <thread>

namespace hi::inline v1 {

/** An unfair mutex
 * This is a fast implementation of a mutex which does not fairly arbitrate
 * between multiple blocking threads. Due to the unfairness it is possible
 * that blocking threads will be completely starved.
 *
 * This mutex however does block on a operating system's futex/unfair_mutex
 * primitives and therefor thread priority are properly handled.
 *
 * On windows and Linux the compiler generally emits the following sequence
 * of instructions:
 *  + non-contented:
 *     - lock(): MOV r,1; XOR r,r; LOCK CMPXCHG; JNE (skip)
 *     - unlock(): LOCK XADD [],-1; CMP; JE
 *
 */
class unfair_rwmutex {
public:
    bool is_locked() const noexcept
    {
         return to_bool(lock_count(_v.load(std::memory_order::relaxed)));
    }

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    [[nodiscard]] hi_force_inline bool exclusive_try_lock() noexcept
    {
        value_type expected = 0;
        if (not _exclusive_try_lock(expected)) {
            [[unlikely]] return exclusive_lock_contended<false>(expected);
        } else {
            return false;
        }
    }

    hi_force_inline void exclusive_lock() noexcept
    {
        value_type expected = 0;
        if (not _exclusive_try_lock(expected)) {
            [[unlikely]] exclusive_lock_contended<true>(expected);
        }
    }

    /** Unlock the mutex.
     *
     * @note It is undefined behavior to call unlock when the thread does not hold the lock.
     */
    hi_force_inline void exclusive_unlock() noexcept
    {
        hi_axiom(holds_invariant());

        if (_v.fetch_sub(exclusive_one, std::memory_order::release) - exclusive_one) {
            hi_axiom(holds_invariant());

            // There can't be any shared-locks outstanding when `fetch_sub()` was called.
            // Which means if any bit is set then there where waiters. Wake one up.
            _v.notify_one();
        }

        hi_axiom(holds_invariant());
    }

    /** Get a shared lock.
     *
     */
    hi_force_inline void shared_lock() noexcept
    {
        value_type expected = 0;
        if (not _shared_try_lock(expected)) {
            [[unlikely]] shared_lock_contended(expected);
        }
    }

    [[nodiscard]] hi_force_inline bool shared_try_lock() noexcept
    {
        value_type expected = 0;
        return _shared_try_lock(expected);
    }

    hi_force_inline void shared_unlock() noexcept
    {
        hi_axiom(holds_invariant());

        hilet tmp = _v.fetch_sub(shared_one, std::memory_order::release) - shared_one;
        if (tmp != 0 and tmp < shared_one) {
            // There can't be any exclusive-locks outstanding when `fetch_sub()` was called.
            // This was also the last shared-lock.
            // Which means if any bit is set then there where waiters. Wake one up.
            _v.notify_one();
        }

        hi_axiom(holds_invariant());
    }

private:
    using atomic_value_type = std::atomic_unsigned_lock_free;
    using value_type = atomic_value_type::value_type;

    constexpr static value_type total_bit = sizeof(value_type) * CHAR_BIT;
    constexpr static value_type waiter_bit = total_bit / 2;
    constexpr static value_type exclusive_bit = 1;
    constexpr static value_type shared_bit = total_bit - exclusive_bit - waiter_bit;

    constexpr static value_type exclusive_off = 0;
    constexpr static value_type shared_off = exclusive_off + exclusive_bit;
    constexpr static value_type waiter_off = shared_off + shared_bit;

    constexpr static value_type exclusive_mask = ((value_type{1} << exclusive_bit) - 1) << exclusive_off;
    constexpr static value_type shared_mask = ((value_type{1} << shared_bit) - 1) << shared_off;
    constexpr static value_type waiter_mask = ((value_type{1} << waiter_bit) - 1) << waiter_off;

    constexpr static value_type exclusive_one = value_type{1} << exclusive_off;
    constexpr static value_type shared_one = value_type{1} << shared_off;
    constexpr static value_type waiter_one = value_type{1} << waiter_off;

    /**
     *
     * The exclusive-count is first, so it can be cut of using shifts when taking
     * a shared-lock.
     *
     * 32 bit value_type:
     *  - [0:0] Number of exclusive locks (can only be one exclusive lock).
     *  - [15:1] Number of shared locks.
     *  - [31:16] Number of waiters.
     *
     * 64 bit value_type:
     *  - [0:0] Number of exclusive locks (can only be one exclusive lock).
     *  - [31:1] Number of shared locks.
     *  - [63:32] Number of waiters.
     */
    atomic_value_type _v = 0;

    [[nodiscard]] static value_type exclusive_count(value_type value) noexcept
    {
        return value & exclusive_mask;
    }

    [[nodiscard]] static value_type shared_count(value_type value) noexcept
    {
        static_assert(waiter_off + waiter_bit == total_bit);
        static_assert(exclusive_off == 0);
        static_assert(shared_off == exclusive_off + exclusive_bit);

        value <<= total_bit - waiter_off;
        value >>= (total_bit - waiter_off) + shared_off;
        return value;
    }

    [[nodiscard]] static bool _is_locked(value_type value) noexcept
    {
        static_assert(waiter_off + waiter_bit == total_bit);

        return to_bool(value << (total_bit - waiter_off));
    }

    [[nodiscard]] static value_type increment_exclusive(value_type value) noexcept
    {
        hi_axiom(value & exclusive_mask != exclusive_mask);
        return value + exclusive_one;
    }

    [[nodiscard]] static value_type increment_shared(value_type value) noexcept
    {
        hi_axiom(value & shared_mask != shared_mask);
        return value + shared_one;
    }

    [[nodiscard]] static value_type increment_wait(value_type value) noexcept
    {
        hi_axiom(value & waiter_mask != waiter_mask);
        return value + waiter_one;
    }

    [[nodiscard]] static value_type clear_exclusive_and_waiter(value_type value) noexcept
    {
        return value & shared_mask;
    }

    [[nodiscard]] bool holds_invariant() const noexcept
    {
        hilet tmp = _v.load(std::memory_order::relaxed);
        return not(to_bool(shared_count(tmp)) and to_bool(exclusive_count(tmp)));
    }

    hi_force_inline void wait(value_type& expected) noexcept
    {
        hi_axiom(holds_invariant());

        // Keep track how many waiters there are.
        hilet desired = increment_wait(expected);
        if (_v.compare_exchange_strong(expected, desired, std::memory_order::relaxed)) {
            hi_axiom(holds_invariant());
            _v.wait(desired);

            // Decrement the wait-count.
            expected = _v.fetch_sub(waiter_one, std::memory_order::relaxed) - waiter_one;
        }

        hi_axiom(holds_invariant());
    }

    [[nodiscard]] hi_force_inline bool _exclusive_try_lock(value_type& expected) noexcept
    {
        hi_axiom(holds_invariant());

        // A non-contended exclusive-lock can only be taken if both the shared-count and exclusive-count are zero.
        // To improve performance also expect the wait-count to be zero, that way we don't need to read the expected value first.
        value_type expected = 0;
        return _v.compare_exchange_strong(
            expected, increment_exclusive(expected), std::memory_order::acquire, std::memory_order::relaxed);
    }

    template<bool Wait>
    hi_no_inline bool exclusive_lock_contended(value_type expected) noexcept
    {
        while (true) {
            hi_axiom(holds_invariant());

            if (_is_locked(expected)) {
                if constexpr (Wait) {
                    wait(expected);
                } else {
                    return false;
                }

            } else {
                if (_v.compare_exchange_strong(
                        expected, increment_exclusive(expected), std::memory_order::acquire, std::memory_order::relaxed)) {
                    hi_axiom(holds_invariant());
                    return true;
                }
            }
        }
    }

    [[nodiscard]] bool _shared_try_lock(value_type& expected) noexcept
    {
        // Increment the shared-count only when exclusive-count and waiter-count are zero.
        expected = clear_exclusive_and_waiter(_v.load(std::memory_order::relaxed));

        return _v.compare_exchange_strong(
            expected, increment_shared(expected), std::memory_order::acquire, std::memory_order::relaxed);
    }

    hi_no_inline void shared_lock_contended(value_type expected) noexcept
    {
        while (true) {
            hi_axiom(holds_invariant());

            if (exclusive_count(expected)) {
                wait(expected);

            } else {
                if (_v.compare_exchange_strong(
                        expected, increment_shared(expected), std::memory_order::acquire, std::memory_order::relaxed)) {
                    hi_axiom(holds_invariant());
                    return;
                }
            }
        }
    }
};

} // namespace hi::inline v1
