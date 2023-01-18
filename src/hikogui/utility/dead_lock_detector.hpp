// Copyright Take Vos 2021.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "assert.hpp"
#include "architecture.hpp"
#include <vector>
#include <algorithm>
#include <compare>

namespace hi::inline v1 {
namespace detail {

struct dead_lock_detector_pair {
    void *before;
    void *after;

    friend bool operator==(dead_lock_detector_pair const &lhs, dead_lock_detector_pair const &rhs) noexcept = default;
    friend std::strong_ordering
    operator<=>(dead_lock_detector_pair const &lhs, dead_lock_detector_pair const &rhs) noexcept = default;
};

} // namespace detail

/** Dead lock detector.
 *
 * The dead lock detector should not be used before and after main().
 * By using `hi::unfair_mutex_impl<false>` you can get a mutex without a dead lock detector.
 *
 * In the future when MSVC allows constinit thread_local variables we can change the implementation
 * to allow the dead lock detector before main().
 */
class dead_lock_detector {
public:
    /** Lock an object on this thread.
     * @param object The object that is being locked.
     * @return nullptr on success, object if the mutex was already locked, a pointer to
     *         another mutex if potential dead-lock is found.
     */
    static void *lock(void *object) noexcept;

    /** Unlock an object on this thread.
     * @param object The object that is being locked.
     * @return true on success, false on failure.
     */
    static bool unlock(void *object) noexcept;

    /** Remove the object from the detection.
     * This function is needed when there are mutex-like objects
     * that are dynamically de-allocated.
     *
     * @param object The object to remove from the lock order graph.
     */
    static void remove_object(void *object) noexcept;

    /** Clear the stack.
     * Is used in unit-tests.
     */
    static void clear_stack() noexcept;

    /** Clear the graph.
     * Is used in unit-tests.
     */
    static void clear_graph() noexcept;

private:
    thread_local inline static std::vector<void *> stack;

    /** The order in which objects where locked.
     * Each pair gives a first before second order.
     *
     * When accessing lock_order dead_lock_detector_mutex must be locked.
     */
    inline static std::vector<detail::dead_lock_detector_pair> lock_graph;

    [[nodiscard]] static void *check_graph(void *object) noexcept;
};

} // namespace hi::inline v1
