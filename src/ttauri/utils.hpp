// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <atomic>

namespace tt::inline v1 {

/** Compare then store if there was a change.
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(T &lhs, U &&rhs) noexcept
{
    if (lhs != rhs) {
        lhs = std::forward<U>(rhs);
        return true;
    } else {
        return false;
    }
}

/** Compare then store if there was a change.
 *
 * @note This atomic version does an lhs.exchange(rhs, std::memory_order_relaxed)
 * @return true if a store was executed.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_store(std::atomic<T> &lhs, U &&rhs) noexcept
{
    return lhs.exchange(rhs, std::memory_order::relaxed) != rhs;
}

} // namespace tt::inline v1