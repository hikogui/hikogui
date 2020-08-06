// Copyright 2019 Pokitec
// All rights reserved.

namespace tt {

/*
 * semaphore value:
 *  0 - Unlocked, no other thread is waiting.
 *  1 - Locked, no other thread is waiting.
 *  2 - Locked, zero or more threads are waiting.
 */

inline unfair_mutex::unfair_mutex() noexcept
{
}

inline unfair_mutex::~unfair_mutex()
{
}

tt_no_inline void unfair_mutex::lock_contented(uint32_t expected) noexcept
{
    do {
        ttlet should_wait = expected == 2;

        // Set to 2 when we are waiting.
        expected = 1;
        if (should_wait || semaphore.compare_exchange_strong(expected, 2)) {

#if TT_BUILD_TYPE == TT_BT_DEBUG
            tt_assert(locking_thread != std::this_thread::get_id());
#endif

            wait_on(semaphore, 2);
        }

        // Set to 2 when acquiring the lock, so that during unlock we wake other waiting threads.
        expected = 0;
    } while (!semaphore.compare_exchange_strong(expected, 2));
}

[[nodiscard]] inline bool unfair_mutex::try_lock() noexcept
{
    // Switch to 1 means there are no waiters.
    uint32_t expected = 0;
    if (tt_unlikely(!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire))) {
        return false;
    }
#if TT_BUILD_TYPE == TT_BT_DEBUG
    locking_thread = std::this_thread::get_id();
#endif
    return true;
}

inline void unfair_mutex::lock() noexcept
{
    // Switch to 1 means there are no waiters.
    uint32_t expected = 0;
    if (tt_unlikely(!semaphore.compare_exchange_strong(expected, 1, std::memory_order::memory_order_acquire))) {
        lock_contented(expected);
    }
#if TT_BUILD_TYPE == TT_BT_DEBUG
    locking_thread = std::this_thread::get_id();
#endif
}

inline void unfair_mutex::unlock() noexcept
{
    if (tt_unlikely(semaphore.fetch_sub(1, std::memory_order::memory_order_relaxed) != 1)) {
        semaphore.store(0, std::memory_order::memory_order_release);

        wake_single_thread_waiting_on(semaphore);
    } else {
        atomic_thread_fence(std::memory_order::memory_order_release);
    }
}

}
