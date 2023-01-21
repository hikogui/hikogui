// Copyright Take Vos 2021-2022.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "dead_lock_detector.hpp"
#include "unfair_mutex.hpp"
#include "thread.hpp"
#include "subsystem.hpp"
#include "../utility/module.hpp"
#include <mutex>

namespace hi::inline v1 {

static unfair_mutex_impl<false> dead_lock_detector_mutex;

[[nodiscard]] void *dead_lock_detector::check_graph(void *object) noexcept
{
    hi_assert_not_null(object);

    hilet lock = std::scoped_lock(dead_lock_detector_mutex);

    for (hilet before : stack) {
        auto correct_order = detail::dead_lock_detector_pair{before, object};
        hilet reverse_order = detail::dead_lock_detector_pair{object, before};

        if (std::binary_search(cbegin(lock_graph), cend(lock_graph), correct_order)) {
            // The object has been locked in the correct order in comparison to `before`.
            continue;
        }

        if (std::binary_search(cbegin(lock_graph), cend(lock_graph), reverse_order)) {
            // The object has been locked in reverse order in comparison to `before`.
            return before;
        }

        // Insert the new 'correct' order in the sorted lock_graph.
        hilet it = std::upper_bound(cbegin(lock_graph), cend(lock_graph), correct_order);
        lock_graph.insert(it, std::move(correct_order));
    }
    return nullptr;
}

void *dead_lock_detector::lock(void *object) noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC after main() returns.
        return nullptr;
    }

    hi_assert_not_null(object);

    for (hilet &x : stack) {
        if (object == x) {
            // `object` already locked by the current thread.
            return object;
        }
    }

    if (auto before = check_graph(object)) {
        // Trying to lock `object` after `before` in previously reversed order
        return before;
    }

    stack.push_back(object);
    return nullptr;
}

/** Unlock an object on this thread.
 */
bool dead_lock_detector::unlock(void *object) noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC when main() returns.
        return true;
    }

    hi_assert_not_null(object);

    // Trying to unlock `object`, but nothing on this thread was locked.
    if (stack.empty()) {
        return false;
    }

    // Trying to unlock `object`, but unlocking in different order.
    if (stack.back() != object) {
        return false;
    }

    stack.pop_back();
    return true;
}

void dead_lock_detector::clear_stack() noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `stack` do not work on MSVC when main() returns.
        return;
    }

    stack.clear();
}

void dead_lock_detector::clear_graph() noexcept
{
    if (is_system_shutting_down()) {
        // thread_local variables used by `lock_graph` do not work on MSVC when main() returns.
        return;
    }

    hilet lock = std::scoped_lock(dead_lock_detector_mutex);
    lock_graph.clear();
}

void dead_lock_detector::remove_object(void *object) noexcept
{
    hi_assert_not_null(object);

    if (is_system_shutting_down()) {
        // thread_local variables used by `lock_graph` do not work on MSVC when main() returns.
        return;
    }

    hilet lock = std::scoped_lock(dead_lock_detector_mutex);

    std::erase_if(lock_graph, [object](hilet &item) {
        return item.before == object or item.after == object;
    });
}

} // namespace hi::inline v1
