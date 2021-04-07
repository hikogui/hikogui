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
        if (std::ranges::any_of(stack, [object](void *x) {
                return object == x;
            })) {
            throw lock_error("object {} already locked by the current thread {} ", reinterpret_cast<ptrdiff_t>(object), thread_id());
        }
    }

    if (auto before = check_graph(object)) {
        // We needed to handle the graph checking in a separate function so that we do not
        // throw while holding the dead_lock_detector_mutex. MSVC will not unwind the stack
        // on a noexcepted throw, and std::abort() will call into the winproc.
        throw lock_error(
            "Trying to lock object {} after {} in thread {}, in previously reversed order",
            reinterpret_cast<ptrdiff_t>(object),
            before,
            thread_id());

    }

    stack.push_back(object);
}

/** Unlock an object on this thread.
 */
void dead_lock_detector::unlock(void *object)
{
    tt_axiom(object != nullptr);

    if (stack.empty()) {
        throw lock_error(
            "Trying to unlock object {}, but nothing on this thread {} was locked.",
            reinterpret_cast<ptrdiff_t>(object),
            thread_id());
    }

    if (stack.back() != object) {
        throw lock_error(
            "Trying to unlock object {}, but this thread {} locked object {} last.",
            reinterpret_cast<ptrdiff_t>(object),
            thread_id(),
            reinterpret_cast<ptrdiff_t>(stack.back()));
    }

    stack.pop_back();
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
