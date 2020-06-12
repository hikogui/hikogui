// Copyright 2019 Pokitec
// All rights reserved.

#pragma once

namespace tt {

/** Assign and compare if there was a change.
 * @return true when the right hand side is different from the left and side.
 */
template<typename T, typename U>
[[nodiscard]] bool assign_and_compare(T &lhs, U &&rhs) noexcept
{
    auto r = (lhs != rhs);
    lhs = std::forward<U>(rhs);
    return r;
}

}