// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {

/** Compare then assign if there was a change.
 * @return true when the right hand side is different from the left and side.
 */
template<typename T, typename U>
[[nodiscard]] bool compare_then_assign(T &lhs, U &&rhs) noexcept
{
    if (lhs != rhs) {
        lhs = std::forward<U>(rhs);
        return true;
    } else {
        return false;
    }
}

}