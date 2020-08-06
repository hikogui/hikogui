// Copyright 2019 Pokitec
// All rights reserved.

#include "unfair_mutex.hpp"
#include "logger.hpp"
#include <Windows.h>
#include <Synchapi.h>

namespace tt {

/*
 * semaphore value:
 *  0 - Unlocked, no other thread is waiting.
 *  1 - Locked, no other thread is waiting.
 *  2 - Locked, zero or more threads are waiting.
 */

unfair_mutex::unfair_mutex() noexcept {}
unfair_mutex::~unfair_mutex() {}

tt_no_inline void unfair_mutex::lock_contented(int32_t expected) noexcept
{
    do {
        ttlet should_wait = expected == 2;

        // Set to 2 when we are waiting.
        expected = 1;
        if (should_wait || semaphore.compare_exchange_strong(expected, 2)) {

#if TT_BUILD_TYPE == TT_BT_DEBUG
            tt_assert(locking_thread != std::this_thread::get_id());
#endif

            // Casting first memory of a struct is allowed.
            expected = 2;
            if (!WaitOnAddress(semaphore_ptr(), &expected, sizeof (int32_t), INFINITE)) {
                LOG_FATAL("Could not wait on address {}", getLastErrorMessage());
            }
        }

        // Set to 2 when aquiring the lock, so that during unlock we wake other waiting threads.
        expected = 0;
    } while (!semaphore.compare_exchange_strong(expected, 2));
}

bool unfair_mutex::try_lock() noexcept
{
    // Switch to 1 means there are no waiters.
    int32_t expected = 0;
    if (tt_unlikely(!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire))) {
        return false;
    }
#if TT_BUILD_TYPE == TT_BT_DEBUG
    locking_thread = std::this_thread::get_id();
#endif
    return true;
}

void unfair_mutex::lock() noexcept
{
    // Switch to 1 means there are no waiters.
    int32_t expected = 0;
    if (tt_unlikely(!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire))) {
        lock_contented(expected);
    }
#if TT_BUILD_TYPE == TT_BT_DEBUG
    locking_thread = std::this_thread::get_id();
#endif
}

void unfair_mutex::unlock() noexcept
{
    if (tt_unlikely(semaphore.fetch_sub(1, std::memory_order::memory_order_relaxed) != 1)) {
        semaphore.store(0, std::memory_order::memory_order_release);

        WakeByAddressSingle(semaphore_ptr());
    } else {
        atomic_thread_fence(std::memory_order::memory_order_release);
    }
}

}
