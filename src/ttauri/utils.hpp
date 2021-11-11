// Copyright Take Vos 2020.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

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

} // namespace tt::inline v1