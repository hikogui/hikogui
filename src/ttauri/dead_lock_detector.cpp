// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include "dead_lock_detector.hpp"
#include "unfair_mutex.hpp"
#include "exception.hpp"
#include "thread.hpp"
#include <mutex>

namespace tt {

static unfair_mutex_impl<false> dead_lock_detector_mutex;

[[nodiscard]] void *dead_lock_detector::check_graph(void *object) noexcept
{
    tt_axiom(object != nullptr);

    ttlet lock = std::scoped_lock(dead_lock_detector_mutex);

    for (ttlet before : stack) {
        for (ttlet order : lock_graph) {
            if (order.first == before && order.second == object) {
                goto next_combination;

            } else if (order.first == object && order.second == before) {
                return before;
            }
        }

        lock_graph.emplace_back(before, object);

next_combination:;
    }
    return nullptr;
}

void dead_lock_detector::lock(void *object, bool recursive_lock)
{
    tt_axiom(object != nullptr);

    if (recursive_lock && !stack.empty() && stack.back() == object) {
        // We are recursive locking and the object was locked last,
        // so we can skip the stack test.
        ;

    } else {
        // `object` already locked by the current thread.
        tt_axiom(not std::ranges::any_of(stack, [object](void *x) {
            return object == x;
        }));
    }

    auto before = check_graph(object);
    // Trying to lock `object` after `before` in previously reversed order
    tt_axiom(before == nullptr);

    stack.push_back(object);
}

/** Unlock an object on this thread.
 */
void dead_lock_detector::unlock(void *object)
{
    tt_axiom(object != nullptr);

    // Trying to unlock `object`, but nothing on this thread was locked.
    tt_axiom(not stack.empty());

    // Trying to unlock `object`, but unlocking in different order.
    tt_axiom(stack.back() == object);

    stack.pop_back();
}

void dead_lock_detector::clear_stack() noexcept
{
    stack.clear();
}

void dead_lock_detector::clear_graph() noexcept
{
    ttlet lock = std::scoped_lock(dead_lock_detector_mutex);
    lock_graph.clear();
}

void dead_lock_detector::remove_object(void *object) noexcept
{
    tt_axiom(object != nullptr);

    ttlet lock = std::scoped_lock(dead_lock_detector_mutex);

    std::erase_if(lock_graph, [object](ttlet &item) {
        return item.first == object || item.second == object;
    });
}

} // namespace tt
