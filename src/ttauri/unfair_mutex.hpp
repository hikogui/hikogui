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
#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
    std::atomic<uint32_t> semaphore = 0;

    tt_no_inline void lock_contented(uint32_t first) noexcept;

#elif TT_OPERATING_SYSTEM == TT_OS_MACOS
    std::unique_ptr<unfair_lock_wrap> mutex;

#else
#error "Not implemented fast_mutex"
#endif

#if TT_BUILD_TYPE == TT_BT_DEBUG
    thread_id locking_thread;
#endif

public:
    unfair_mutex(unfair_mutex const &) = delete;
    unfair_mutex &operator=(unfair_mutex const &) = delete;

    unfair_mutex() noexcept;
    ~unfair_mutex();

    void lock() noexcept;

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    bool try_lock() noexcept;

    void unlock() noexcept;
};

}

#if TT_OPERATING_SYSTEM == TT_OS_WINDOWS
#include "unfair_mutex_win32_impl.hpp"
#elif TT_OPERATING_SYSTEM == TT_OS_MACOS
#include "unfair_mutex_macos_impl.hpp"
#else
#error "unfair_mutex not implemented for this operating system."
#endif
