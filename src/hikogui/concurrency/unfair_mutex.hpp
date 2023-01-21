// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/unfair_mutex.hpp Definition of the unfair_mutex.
 * @ingroup concurrency
 */

#pragma once

#include "dead_lock_detector.hpp"
#include "../utility/module.hpp"
#include <atomic>
#include <memory>

namespace hi { inline namespace v1 {

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
 * @ingroup concurrency
 * @tparam UseDeadLockDetector true when the unfair_mutex will use the deadlock detector.
 */
template<bool UseDeadLockDetector>
class unfair_mutex_impl {
public:
    constexpr unfair_mutex_impl() noexcept {}
    unfair_mutex_impl(unfair_mutex_impl const&) = delete;
    unfair_mutex_impl(unfair_mutex_impl&&) = delete;
    unfair_mutex_impl& operator=(unfair_mutex_impl const&) = delete;
    unfair_mutex_impl& operator=(unfair_mutex_impl&&) = delete;

    ~unfair_mutex_impl()
    {
        if constexpr (UseDeadLockDetector) {
            dead_lock_detector::remove_object(this);
        }
    }

    bool is_locked() const noexcept
    {
        return semaphore.load(std::memory_order::relaxed) != 0;
    }

    void lock() noexcept
    {
        if constexpr (UseDeadLockDetector) {
            hilet other = dead_lock_detector::lock(this);
            // *this mutex is already locked.
            hi_assert(other != this);
            // Potential dead-lock because of different ordering with other.
            hi_assert(other == nullptr);
        }

        hi_axiom(holds_invariant());

        // Switch to 1 means there are no waiters.
        semaphore_value_type expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
            [[unlikely]] lock_contended(expected);
        }

        hi_axiom(holds_invariant());
    }

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    [[nodiscard]] bool try_lock() noexcept
    {
        if constexpr (UseDeadLockDetector) {
            hilet other = dead_lock_detector::lock(this);
            // *this mutex is already locked.
            hi_assert(other != this);
            // Potential dead-lock because of different ordering with other.
            hi_assert(other == nullptr);
        }

        hi_axiom(holds_invariant());

        // Switch to 1 means there are no waiters.
        semaphore_value_type expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
            hi_axiom(holds_invariant());

            if constexpr (UseDeadLockDetector) {
                // *this mutex is locked out-of-order from the order of being locked.
                hi_assert(dead_lock_detector::unlock(this));
            }

            [[unlikely]] return false;
        }

        hi_axiom(holds_invariant());
        return true;
    }

    void unlock() noexcept
    {
        if constexpr (UseDeadLockDetector) {
            // *this mutex is locked out-of-order from the order of being locked.
            hi_assert(dead_lock_detector::unlock(this));
        }

        hi_axiom(holds_invariant());

        if (semaphore.fetch_sub(1, std::memory_order::relaxed) != 1) {
            [[unlikely]] semaphore.store(0, std::memory_order::release);

            semaphore.notify_one();
        } else {
            atomic_thread_fence(std::memory_order::release);
        }

        hi_axiom(holds_invariant());
    }

private:
    /*
     * semaphore value:
     *  0 - Unlocked, no other thread is waiting.
     *  1 - Locked, no other thread is waiting.
     *  2 - Locked, zero or more threads are waiting.
     */
    std::atomic_unsigned_lock_free semaphore = 0;
    using semaphore_value_type = typename decltype(semaphore)::value_type;

    bool holds_invariant() const noexcept
    {
        return semaphore.load(std::memory_order::relaxed) <= 2;
    }

    hi_no_inline void lock_contended(semaphore_value_type expected) noexcept
    {
        hi_axiom(holds_invariant());

        do {
            hilet should_wait = expected == 2;

            // Set to 2 when we are waiting.
            expected = 1;
            if (should_wait || semaphore.compare_exchange_strong(expected, 2)) {
                hi_axiom(holds_invariant());
                semaphore.wait(2);
            }

            hi_axiom(holds_invariant());
            // Set to 2 when acquiring the lock, so that during unlock we wake other waiting threads.
            expected = 0;
        } while (!semaphore.compare_exchange_strong(expected, 2));
    }
};

#ifndef NDEBUG
using unfair_mutex = unfair_mutex_impl<true>;
#else
using unfair_mutex = unfair_mutex_impl<false>;
#endif

}} // namespace hi::v1
