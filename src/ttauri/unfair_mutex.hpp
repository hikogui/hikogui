// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "required.hpp"
#include "thread.hpp"
#include "assert.hpp"
#include "dead_lock_detector.hpp"
#include <atomic>
#include <memory>
#include <thread>

namespace tt {

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
template<bool UseDeadLockDetector>
class unfair_mutex_impl {
public:
    unfair_mutex_impl() noexcept {}
    unfair_mutex_impl(unfair_mutex_impl const &) = delete;
    unfair_mutex_impl(unfair_mutex_impl &&) = delete;
    unfair_mutex_impl &operator=(unfair_mutex_impl const &) = delete;
    unfair_mutex_impl &operator=(unfair_mutex_impl &&) = delete;

    ~unfair_mutex_impl() requires (UseDeadLockDetector)
    {
        dead_lock_detector::remove_object(this);
    }

    ~unfair_mutex_impl() = default;

    void lock() noexcept
    {
        if constexpr (UseDeadLockDetector) {
            ttlet other = dead_lock_detector::lock(this);
            tt_axiom(other != this, "Mutex already locked.");
            tt_axiom(other == nullptr, "Potential dead-lock.");
        }

        tt_axiom(semaphore.load() <= 2);

        // Switch to 1 means there are no waiters.
        uint32_t expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
            [[unlikely]] lock_contented(expected);
        }

        tt_axiom(semaphore.load() <= 2);
    }

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    [[nodiscard]] bool try_lock() noexcept {
        if constexpr (UseDeadLockDetector) {
            ttlet other = dead_lock_detector::lock(this);
            tt_axiom(other != this, "Mutex already locked.");
            tt_axiom(other == nullptr, "Potential dead-lock.");
        }

        tt_axiom(semaphore.load() <= 2);

        // Switch to 1 means there are no waiters.
        uint32_t expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
            tt_axiom(semaphore.load() <= 2);

            if constexpr (UseDeadLockDetector) {
                tt_axiom(dead_lock_detector::unlock(this), "Unlocking mutex out of order.");
            }

            [[unlikely]] return false;
        }

        tt_axiom(semaphore.load() <= 2);
        return true;
    }

    void unlock() noexcept {
        if constexpr (UseDeadLockDetector) {
            tt_axiom(dead_lock_detector::unlock(this), "Unlocking mutex out of order.");
        }

        tt_axiom(semaphore.load() <= 2);

        if (semaphore.fetch_sub(1, std::memory_order::relaxed) != 1) {
            [[unlikely]] semaphore.store(0, std::memory_order::release);

            semaphore.notify_one();
        } else {
            atomic_thread_fence(std::memory_order::release);
        }

        tt_axiom(semaphore.load() <= 2);
    }

private:
    /*
     * semaphore value:
     *  0 - Unlocked, no other thread is waiting.
     *  1 - Locked, no other thread is waiting.
     *  2 - Locked, zero or more threads are waiting.
     */
    std::atomic<uint32_t> semaphore = 0;

    tt_no_inline void lock_contented(uint32_t expected) noexcept
    {
        tt_axiom(semaphore.load() <= 2);

        do {
            ttlet should_wait = expected == 2;

            // Set to 2 when we are waiting.
            expected = 1;
            if (should_wait || semaphore.compare_exchange_strong(expected, 2)) {
                tt_axiom(semaphore.load() <= 2);
                semaphore.wait(2);
            }

            tt_axiom(semaphore.load() <= 2);
            // Set to 2 when acquiring the lock, so that during unlock we wake other waiting threads.
            expected = 0;
        } while (!semaphore.compare_exchange_strong(expected, 2));
    }
};

#if TT_BUILD_TYPE == TT_BT_DEBUG
using unfair_mutex = unfair_mutex_impl<true>;
#else
using unfair_mutex = unfair_mutex_impl<false>;
#endif

}
