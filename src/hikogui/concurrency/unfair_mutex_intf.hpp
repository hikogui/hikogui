// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/unfair_mutex.hpp Definition of the unfair_mutex.
 * @ingroup concurrency
 */

#pragma once

#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <atomic>
#include <memory>

hi_export_module(hikogui.concurrency.unfair_mutex : intf);

hi_export namespace hi { inline namespace v1 {

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

    ~unfair_mutex_impl();

    bool is_locked() const noexcept;

    void lock() noexcept;

    /**
     * When try_lock() is called from a thread that already owns the lock it will
     * return false.
     *
     * Calling try_lock() in a loop will bypass the operating system's wait system,
     * meaning that no priority inversion will take place.
     */
    [[nodiscard]] bool try_lock() noexcept;

    void unlock() noexcept;

private:
    /*
     * semaphore value:
     *  0 - Unlocked, no other thread is waiting.
     *  1 - Locked, no other thread is waiting.
     *  2 - Locked, zero or more threads are waiting.
     */
    std::atomic_unsigned_lock_free semaphore = 0;
    using semaphore_value_type = typename decltype(semaphore)::value_type;

    bool holds_invariant() const noexcept;

    void lock_contended(semaphore_value_type expected) noexcept;
};

#ifndef NDEBUG
using unfair_mutex = unfair_mutex_impl<true>;
#else
using unfair_mutex = unfair_mutex_impl<false>;
#endif

}} // namespace hi::v1
