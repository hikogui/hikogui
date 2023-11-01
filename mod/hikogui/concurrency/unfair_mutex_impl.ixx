// Copyright Take Vos 2020-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

/** @file concurrency/unfair_mutex.hpp Definition of the unfair_mutex.
 * @ingroup concurrency
 */

module;
#include "../macros.hpp"

#include <atomic>
#include <memory>
#include <format>
#include <mutex>
#include <vector>
#include <algorithm>

export module hikogui_concurrency_unfair_mutex : impl;
import : intf;
import hikogui_concurrency_global_state;
import hikogui_utility;

export namespace hi { inline namespace v1 {
namespace detail {

unfair_mutex_impl<false> unfair_mutex_deadlock_mutex;

thread_local std::vector<void *> unfair_mutex_deadlock_stack;

/** The order in which objects where locked.
 * Each pair gives a first before second order.
 *
 * When accessing lock_order unfair_mutex_deadlock_mutex must be locked.
 */
std::vector<std::pair<void *, void *>> unfair_mutex_deadlock_lock_graph;

[[nodiscard]] void *unfair_mutex_deadlock_check_graph(void *object) noexcept
{
    hi_assert_not_null(object);

    hilet lock = std::scoped_lock(detail::unfair_mutex_deadlock_mutex);

    for (hilet before : unfair_mutex_deadlock_stack) {
        auto correct_order = std::make_pair(before, object);
        hilet reverse_order = std::make_pair(object, before);

        if (std::binary_search(
                unfair_mutex_deadlock_lock_graph.cbegin(), unfair_mutex_deadlock_lock_graph.cend(), correct_order)) {
            // The object has been locked in the correct order in comparison to `before`.
            continue;
        }

        if (std::binary_search(
                unfair_mutex_deadlock_lock_graph.cbegin(), unfair_mutex_deadlock_lock_graph.cend(), reverse_order)) {
            // The object has been locked in reverse order in comparison to `before`.
            return before;
        }

        // Insert the new 'correct' order in the sorted lock_graph.
        hilet it =
            std::upper_bound(unfair_mutex_deadlock_lock_graph.cbegin(), unfair_mutex_deadlock_lock_graph.cend(), correct_order);
        unfair_mutex_deadlock_lock_graph.insert(it, std::move(correct_order));
    }
    return nullptr;
}

} // namespace detail

/** Lock an object on this thread.
 * @param object The object that is being locked.
 * @return nullptr on success, object if the mutex was already locked, a pointer to
 *         another mutex if potential dead-lock is found.
 */
export void *unfair_mutex_deadlock_lock(void *object) noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC after main() returns.
        return nullptr;
    }

    hi_assert_not_null(object);

    if (std::count(detail::unfair_mutex_deadlock_stack.begin(), detail::unfair_mutex_deadlock_stack.end(), object) != 0) {
        // `object` already locked by the current thread.
        return object;
    }

    if (auto before = detail::unfair_mutex_deadlock_check_graph(object)) {
        // Trying to lock `object` after `before` in previously reversed order
        return before;
    }

    detail::unfair_mutex_deadlock_stack.push_back(object);
    return nullptr;
}

/** Unlock an object on this thread.
 * @param object The object that is being locked.
 * @return true on success, false on failure.
 */
export bool unfair_mutex_deadlock_unlock(void *object) noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC when main() returns.
        return true;
    }

    hi_assert_not_null(object);

    // Trying to unlock `object`, but nothing on this thread was locked.
    if (detail::unfair_mutex_deadlock_stack.empty()) {
        return false;
    }

    // Trying to unlock `object`, but unlocking in different order.
    if (detail::unfair_mutex_deadlock_stack.back() != object) {
        return false;
    }

    detail::unfair_mutex_deadlock_stack.pop_back();
    return true;
}

/** Remove the object from the detection.
 * This function is needed when there are mutex-like objects
 * that are dynamically de-allocated.
 *
 * @param object The object to remove from the lock order graph.
 */
export void unfair_mutex_deadlock_remove_object(void *object) noexcept
{
    hi_assert_not_null(object);

    if (is_system_shutting_down()) {
        // thread_local variables used by `lock_graph` do not work on MSVC when main() returns.
        return;
    }

    hilet lock = std::scoped_lock(detail::unfair_mutex_deadlock_mutex);
    std::erase_if(detail::unfair_mutex_deadlock_lock_graph, [object](hilet& item) {
        return item.first == object or item.second == object;
    });
}

/** Clear the stack.
 * Is used in unit-tests.
 */
export void unfair_mutex_deadlock_clear_stack() noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC when main() returns.
        return;
    }

    detail::unfair_mutex_deadlock_stack.clear();
}

/** Clear the graph.
 * Is used in unit-tests.
 */
export void unfair_mutex_deadlock_clear_graph() noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `lock_graph` do not work on MSVC when main() returns.
        return;
    }

    hilet lock = std::scoped_lock(detail::unfair_mutex_deadlock_mutex);
    detail::unfair_mutex_deadlock_lock_graph.clear();
}

template<bool UseDeadLockDetector>
unfair_mutex_impl<UseDeadLockDetector>::~unfair_mutex_impl()
{
    hi_axiom(not is_locked());
    if constexpr (UseDeadLockDetector) {
        unfair_mutex_deadlock_remove_object(this);
    }
}

template<bool UseDeadLockDetector>
bool unfair_mutex_impl<UseDeadLockDetector>::is_locked() const noexcept
{
    return semaphore.load(std::memory_order::relaxed) != 0;
}

template<bool UseDeadLockDetector>
void unfair_mutex_impl<UseDeadLockDetector>::lock() noexcept
{
    if constexpr (UseDeadLockDetector) {
        hilet other = unfair_mutex_deadlock_lock(this);
        hi_assert(other != this, "This mutex is already locked.");
        hi_assert(other == nullptr, "Potential dead-lock because of different lock ordering of mutexes.");
    }

    hi_axiom(holds_invariant());

    // Switch to 1 means there are no waiters.
    semaphore_value_type expected = 0;
    if (not semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
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
template<bool UseDeadLockDetector>
[[nodiscard]] bool unfair_mutex_impl<UseDeadLockDetector>::try_lock() noexcept
{
    if constexpr (UseDeadLockDetector) {
        hilet other = unfair_mutex_deadlock_lock(this);
        hi_assert(other != this, "This mutex is already locked.");
        hi_assert(other == nullptr, "Potential dead-lock because of different lock ordering of mutexes.");
    }

    hi_axiom(holds_invariant());

    // Switch to 1 means there are no waiters.
    semaphore_value_type expected = 0;
    if (not semaphore.compare_exchange_strong(expected, 1, std::memory_order::acquire)) {
        hi_axiom(holds_invariant());

        if constexpr (UseDeadLockDetector) {
            hi_assert(unfair_mutex_deadlock_unlock(this), "Unlock is not done in reverse order.");
        }

        [[unlikely]] return false;
    }

    hi_axiom(holds_invariant());
    return true;
}

template<bool UseDeadLockDetector>
void unfair_mutex_impl<UseDeadLockDetector>::unlock() noexcept
{
    if constexpr (UseDeadLockDetector) {
        hi_assert(unfair_mutex_deadlock_unlock(this), "Unlock is not done in reverse order.");
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

template<bool UseDeadLockDetector>
[[nodiscard]] bool unfair_mutex_impl<UseDeadLockDetector>::holds_invariant() const noexcept
{
    return semaphore.load(std::memory_order::relaxed) <= 2;
}

template<bool UseDeadLockDetector>
hi_no_inline void unfair_mutex_impl<UseDeadLockDetector>::lock_contended(semaphore_value_type expected) noexcept
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

}} // namespace hi::v1
