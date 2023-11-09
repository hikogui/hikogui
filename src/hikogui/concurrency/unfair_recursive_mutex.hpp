// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/unfair_recursive_mutex.hpp Definition of the unfair_recursive_mutex.
 * @ingroup concurrency
 */

#pragma once

#include "unfair_mutex.hpp"
#include "thread.hpp"
#include "../utility/utility.hpp"
#include "../macros.hpp"
#include <thread>
#include <atomic>

hi_export_module(hikogui.concurrency.unfair_recursive_mutex);

hi_export namespace hi { inline namespace v1 {

/** An unfair recursive-mutex
 * This is a fast implementation of a recursive-mutex which does not fairly
 * arbitrate between multiple blocking threads. Due to the unfairness it is
 * possible that blocking threads will be completely starved.
 *
 * This recursive-mutex however does block on a operating system's
 * futex/unfair_mutex primitives and therefor thread priority are properly
 * handled.
 *
 * On windows and Linux the compiler generally emits the following sequence
 * of instructions:
 *  + non-recursive:
 *     - lock(): LEA, 2*MOV r,[]; CMP; JNE; MOV r,#; unfair_mutex.lock(); 2*MOV [],r
 *     - unlock(): ADD [],-1; JNE (skip); XOR r,r; MOV [],r; unfair_mutex.unlock()
 *  + recursive:
 *     - lock(): LEA, 2*MOV r,[]; CMP; JNE (skip); LEA, INC [], JMP
 *     - unlock(): ADD [],-1; JNE
 *
 * @ingroup concurrency
 */
class unfair_recursive_mutex {
    /* Thread annotation syntax.
     *
     * FIRST - The thread that acquires/acquired the mutex
     * OWNER - The FIRST thread that recursively requests a lock.
     * OTHER - Another thread while the mutex is held.
     */

    unfair_mutex_impl<false> mutex;

    // FIRST=write, OWNER|OTHER=read
    std::atomic<thread_id> owner = 0;

    // FIRST=write, OWNER=increment, FIRST|OWNER=decrement
    uint32_t count = 0;

public:
    unfair_recursive_mutex(unfair_recursive_mutex const&) = delete;
    unfair_recursive_mutex& operator=(unfair_recursive_mutex const&) = delete;

    unfair_recursive_mutex() = default;
    ~unfair_recursive_mutex() = default;

    /** This function should be used in hi_axiom() to check if the lock is held by current thread.
     *
     * @return The number of recursive locks the current thread has taken.
     * @retval 0 The current thread does not have a lock, or no-thread have a lock.
     */
    [[nodiscard]] int recurse_lock_count() const noexcept
    {
        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        //
        // This only works for comparing the owner with the current thread, it would
        // not work to check the owner with a thread_id of another thread.
        if (owner.load(std::memory_order::acquire) == current_thread_id()) {
            return count;
        } else {
            return 0;
        }
    }

    /**
     * When `try_lock()` is called on a thread that already holds the lock true is returned.
     */
    [[nodiscard]] bool try_lock() noexcept
    {
        // FIRST | OWNER | OTHER
        hilet thread_id = current_thread_id();

        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        //
        // note: theoretically a relaxed load could be enough, but in C++20 any undefined behaviour causing an out-of-bound
        //       array access inside the critical section protected by the unfair_recursive_mutex will overwrite owner
        //       from the point of view of the optimizer.
        if (owner.load(std::memory_order::acquire) == thread_id) {
            // FIRST | OWNER
            hi_axiom(count != 0);
            ++count;

            // OWNER
            return true;

        } else if (mutex.try_lock()) { // OTHER (inside the if expression)
            // FIRST
            hi_axiom(count == 0);
            count = 1;
            hi_axiom(owner == 0);

            // note: theoretically a relaxed store could be enough, but in C++20 any undefined behaviour causing an out-of-bound
            //       array access inside the critical section protected by the unfair_recursive_mutex will overwrite owner
            //       from the point of view of the optimizer.
            owner.store(thread_id, std::memory_order::release);

            return true;

        } else {
            // OTHER
            return false;
        }
    }

    /**
     *
     *                lea  rbx,[rcx+98h]
     *                mov  esi,dword ptr gs:[48h]
     *                mov  eax,dword ptr [rbx+4]
     *                cmp  eax,esi
     *                jne  non_recursive
     *                lea  r15,[rbx+8]
     *                inc  dword ptr [r15]
     *                jmp  locked
     * non_recursive: call unfair_mutex.lock()
     *                lea  r15,[r14+0A0h]
     *                mov  dword ptr [r15],r13d
     *                mov  dword ptr [rbx+4],esi
     * locked:
     */
    void lock() noexcept
    {
        // FIRST | OWNER | OTHER
        hilet thread_id = current_thread_id();

        // The following load() is:
        // - valid-and-equal to thread_id when the OWNER has the lock.
        // - zero or valid-and-not-equal to thread_id when this is an OTHER thread.
        //
        // note: theoretically a relaxed load could be enough, but in C++20 any undefined behaviour causing an out-of-bound
        //       array access inside the critical section protected by the unfair_recursive_mutex will overwrite owner
        //       from the point of view of the optimizer.
        if (owner.load(std::memory_order::acquire) == thread_id) {
            // FIRST | OWNER
            hi_axiom(count != 0);
            ++count;

            // OWNER

        } else {
            // OTHER
            mutex.lock();

            // FIRST
            hi_axiom(count == 0);
            count = 1;
            hi_axiom(owner == 0);

            // note: theoretically a relaxed store could be enough, but in C++20 any undefined behaviour causing an out-of-bound
            //       array access inside the critical section protected by the unfair_recursive_mutex will overwrite owner
            //       from the point of view of the optimizer.
            owner.store(thread_id, std::memory_order::release);
        }
    }

    void unlock() noexcept
    {
        // FIRST | OWNER

        // Unlock must be called on the thread that locked the mutex
        hi_axiom(recurse_lock_count());

        if (--count == 0) {
            // FIRST

            // Only OTHER can execute in `lock()` or `try_lock()`,
            // where it will either see the thread_id of FIRST or zero.
            // In both cases the OTHER thread is detected correctly.
            owner.store(0, std::memory_order::release);

            mutex.unlock();
            // OTHER
        }
        // OWNER | OTHER
    }
};

}} // namespace hi::v1
