// Copyright 2020 Pokitec
// All rights reserved.

#pragma once

#include "required.hpp"
#include "thread.hpp"
#include <atomic>
#include <memory>
#include <thread>

namespace tt {
struct unfair_lock_wrap;

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
class unfair_mutex {
public:
    unfair_mutex() noexcept {}
    unfair_mutex(unfair_mutex const &) = delete;
    unfair_mutex(unfair_mutex &&) = delete;
    unfair_mutex &operator=(unfair_mutex const &) = delete;
    unfair_mutex &operator=(unfair_mutex &&) = delete;

    void lock() noexcept
    {
        // Switch to 1 means there are no waiters.
        uint32_t expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire)) {
            [[unlikely]] lock_contented(expected);
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        locking_thread = current_thread_id();
#endif
    }

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    bool try_lock() noexcept {
        // Switch to 1 means there are no waiters.
        uint32_t expected = 0;
        if (!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire)) {
            [[unlikely]] return false;
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        locking_thread = current_thread_id();
#endif
        return true;
    }

    void unlock() noexcept {
        if (semaphore.fetch_sub(1, std::memory_order::memory_order_relaxed) != 1) {
            [[unlikely]] semaphore.store(0, std::memory_order::memory_order_release);

            semaphore.notify_one();
        } else {
            atomic_thread_fence(std::memory_order::memory_order_release);
        }
#if TT_BUILD_TYPE == TT_BT_DEBUG
        locking_thread = 0;
#endif
    }

private:
    /*
     * semaphore value:
     *  0 - Unlocked, no other thread is waiting.
     *  1 - Locked, no other thread is waiting.
     *  2 - Locked, zero or more threads are waiting.
     */
    std::atomic<uint32_t> semaphore = 0;

#if TT_BUILD_TYPE == TT_BT_DEBUG
    thread_id locking_thread;
#endif

    tt_no_inline void lock_contented(uint32_t expected) noexcept
    {
        do {
            ttlet should_wait = expected == 2;

            // Set to 2 when we are waiting.
            expected = 1;
            if (should_wait || semaphore.compare_exchange_strong(expected, 2)) {
#if TT_BUILD_TYPE == TT_BT_DEBUG
                // This check only works because locking_thread can never be the current
                // thread id. It is either the thread that made the lock, or it is zero.
                tt_assert(locking_thread != current_thread_id());
#endif
                semaphore.wait(2);
            }

            // Set to 2 when acquiring the lock, so that during unlock we wake other waiting threads.
            expected = 0;
        } while (!semaphore.compare_exchange_strong(expected, 2));
    }
};

}
